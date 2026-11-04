// TODO: implement neighbor checks and only render exposed blocks

#include <atomic>
#include <functional>
#include <unordered_map>
#include <queue>

#include "chunk.h"
#include "camera.h"

template<>
struct std::hash<ChunkPos> {

  std::size_t operator()(const ChunkPos &chunkPos) const {
    using std::hash;
    return ((hash<int64_t>()(chunkPos.x)
          ^ (hash<int64_t>()(chunkPos.y) << 1)) >> 1)
          ^ (hash<int64_t>()(chunkPos.z) << 1);
  }

};

class World {
public:
  World(const Shader &customShader, const Camera &camera) : customShader(customShader), camera(camera) {}

  // queue all chunks within render distance of the player to be loaded if they are not already
  void queueChunkLoading(void) {

    // loop forever as long as the world needs to be generated
    while (shouldGenerateWorld) {
      // grab the current player/camera position
      float camX = camera.cameraPos.x;
      float camY = camera.cameraPos.y;
      float camZ = camera.cameraPos.z;

      // define a search radius around the current position
      int radius = renderDistance * 16;

      // use integer truncation to store the relative chunk coordinate for the position being searched
      int chunkX, chunkY, chunkZ;

      // loop through a 3d sphere around the current position
      for (float px = camX - radius; px <= camX + radius; px++) {
        for (float py = camY - radius; py <= camY + radius; py++) {
          for (float pz = camZ - radius; pz <= camZ + radius; pz++) {

            // check if the position being checked is within the search radius
            float dx = camX - px, dy = camY - py, dz = camZ - pz;
            if (dx * dx + dy * dy + dz * dz <= radius * radius * radius) {

              // calculate normalized chunk coordinates and use them to generate a target chunk
              chunkX = px / 16;
              chunkY = py / 16;
              chunkZ = pz / 16;
              ChunkPos targetChunkPos = {chunkX, chunkY, chunkZ};

              // check if the target chunk exists yet
              std::unordered_map<ChunkPos, Chunk>::iterator targetChunk = Chunks.find(targetChunkPos);

              // runs if the target chunk does not yet exist
              if (targetChunk == Chunks.end()){

                // generate the target chunk and add it to the Chunks unordered_map
                Chunks.emplace(targetChunkPos, Chunk(targetChunkPos, customShader));

                // find where the new chunk was added to the map, and pass a pointer to that location to the load queue
                std::unordered_map<ChunkPos, Chunk>::iterator freshChunk = Chunks.find(targetChunkPos);
                terrainGenerationQueue.push(&freshChunk->second);

              // if the target chunk does exist, then update the neighbors array for the chunks around it and regenerate its mesh
              } else {
                updateNeighbors(targetChunk->second);

                if (meshGenerationQueue.empty()) {
                  meshGenerationQueue.push(&targetChunk->second);
                }
              }
            }
          }
        }
      }
    }

    return;
  }

  // take an element from the terrain generation queue, generate it, and then send it to the mesh generation queue
  void processTerrainGenerationQueue(void) {
    while(shouldGenerateWorld) {
      if (!terrainGenerationQueue.empty()) {
        terrainGenerationQueue.front()->generateChunkState();
        meshGenerationQueue.push(terrainGenerationQueue.front());
        terrainGenerationQueue.pop();
      }
    }

    return;
  }

  // take an element from the mesh generation queue, generate it, and then send it to the gpu buffer queue
  void processMeshGenerationQueue(void) {
    while(shouldGenerateWorld) {
      if (!meshGenerationQueue.empty()) {
        meshGenerationQueue.front()->generateChunkMesh();
        bufferQueue.push(meshGenerationQueue.front());
        meshGenerationQueue.pop();
      }
    }

    return;
  }

  // take an element from the buffer queue, and buffer its data to the gpu
  void processBufferQueue(unsigned int count) {
    for (unsigned i = 0; i < count; i++) {
      if (!bufferQueue.empty()) {
        bufferQueue.front()->bufferChunkData();
        bufferQueue.pop();
      }
    }

    return;
  }

  // loop through the unordered map and render it all
  void renderWorld(void) {
    for (std::unordered_map<ChunkPos, Chunk>::const_iterator it = Chunks.begin(); it != Chunks.end(); it++) { 
      // make sure the chunk is initialized before trying to load it (this is important if it hasn't been processed through the queue yet)
      if (it->second.chunkIsLoaded()) {
        // get the second element of the unordered map (so the chunk itself, not the hash)
        it->second.renderChunkMesh();
      }
    }

    return;
  }

  // return a neighboring chunk in a specified direction if it exists
  Chunk* getNeighbor(const Chunk& chunk, Direction direction) {
    ChunkPos currentChunkPos = chunk.getChunkPos();
    ChunkPos targetChunkPos;

    // generate a position for where to search based on the current chunk position and the search direction
    switch (direction) {
      case NORTH:
        targetChunkPos = {currentChunkPos.x, currentChunkPos.y, currentChunkPos.z + 1};
        break;
      case SOUTH:
        targetChunkPos = {currentChunkPos.x, currentChunkPos.y, currentChunkPos.z - 1};
        break;
      case EAST:
        targetChunkPos = {currentChunkPos.x + 1, currentChunkPos.y, currentChunkPos.z};
        break;
      case WEST:
        targetChunkPos = {currentChunkPos.x - 1, currentChunkPos.y, currentChunkPos.z};
        break;
      case ABOVE:
        targetChunkPos = {currentChunkPos.x, currentChunkPos.y + 1, currentChunkPos.z};
        break;
      case BELOW:
        targetChunkPos = {currentChunkPos.x, currentChunkPos.y - 1, currentChunkPos.z};
        break;
      // if an invald enum is passed, return a nullptr since this is indefined behavior (and should never happen)
      default:
        return nullptr;
    }

    // check if the target chunk exists within the chunk map
    std::unordered_map<ChunkPos, Chunk>::iterator targetChunk = Chunks.find(targetChunkPos);

    // if it does, return a pointer to it
    if (targetChunk != Chunks.end()){
      return &targetChunk->second;
    // if it is not found, return a nullptr
    } else {
      return nullptr;
    }

  }

  // update the neighrs array for a chunk with all of its neighboring chunks
  void updateNeighbors(Chunk& chunk) {
    chunk.setNeighbor( getNeighbor(chunk, NORTH), NORTH ); 
    chunk.setNeighbor( getNeighbor(chunk, SOUTH), NORTH ); 
    chunk.setNeighbor( getNeighbor(chunk,  EAST),  WEST ); 
    chunk.setNeighbor( getNeighbor(chunk,  WEST),  EAST ); 
    chunk.setNeighbor( getNeighbor(chunk, ABOVE), ABOVE ); 
    chunk.setNeighbor( getNeighbor(chunk, BELOW), BELOW ); 

    return;
  }

  void closeWorld(void) {
    shouldGenerateWorld = false;
  }

private:
  const Shader &customShader;
  const Camera &camera;

  const int renderDistance = 4;

  std::atomic<bool> shouldGenerateWorld = true;

  std::unordered_map<ChunkPos, Chunk> Chunks; 
  std::queue<Chunk*> terrainGenerationQueue;
  std::queue<Chunk*> meshGenerationQueue;
  std::queue<Chunk*> bufferQueue;
};
