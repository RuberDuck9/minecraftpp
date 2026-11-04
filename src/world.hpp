// TODO: implement neighbor checks and only render exposed blocks

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

  // generate a new chunk and buffer its data to the gpu
  void generateChunk(Chunk &newChunk) const {
    newChunk.initializeChunk();
    newChunk.generateChunkState();
    newChunk.generateChunkMesh();
    newChunk.bufferChunkData();
  }

  // queue all chunks within render distance of the player to be loaded if they are not already
  void queueChunkLoading(void) {

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
            std::unordered_map<ChunkPos, Chunk>::const_iterator targetChunk = Chunks.find(targetChunkPos);

            // runs if the target chunk does not yet exist
            if (targetChunk == Chunks.end()){

              // generate the target chunk and add it to the Chunks unordered_map
              Chunks.emplace(targetChunkPos, Chunk(targetChunkPos, customShader));

              // find where the new chunk was added to the map, and pass a pointer to that location to the load queue
              std::unordered_map<ChunkPos, Chunk>::iterator freshChunk = Chunks.find(targetChunkPos);
              loadQueue.push(&freshChunk->second);
            }
          }
        }
      }
    }

    return;
  }

  // take n elements out of the loadQueue, generate them, and buffer them to the gpu
  void processLoadQueue(unsigned int count) {
    for (unsigned int i = 0; i < count; i++) {
      if (!loadQueue.empty()) {
        generateChunk(*loadQueue.front());
        loadQueue.pop();
      }
    }

    return;
  }

  // loop through the unordered map and render it all
  void renderWorld(void) {
    for (std::unordered_map<ChunkPos, Chunk>::const_iterator it = Chunks.begin(); it != Chunks.end(); it++) { 
      // make sure the chunk is initialized before trying to load it (this is important if it hasn't been processed through the queue yet)
      if (it->second.chunkIsInitialized()) {
        // get the second element of the unordered map (so the chunk itself, not the hash)
        it->second.renderChunkMesh();
      }
    }

    return;
  }


private:
  const Shader &customShader;
  const Camera &camera;

  const int renderDistance = 4;

  std::unordered_map<ChunkPos, Chunk> Chunks; 
  std::queue<Chunk*> loadQueue;
};
