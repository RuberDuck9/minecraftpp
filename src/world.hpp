// TODO: implement neighbor checks and only render exposed blocks *******
// TODO: update the neighbor update function to return false if no neighbors needed updating

#include <GLFW/glfw3.h>
#include <atomic>
#include <cmath>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <deque>
#include <algorithm>

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

  // THREAD 1 - queue all chunks within render distance of the player to be loaded if they are not already
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
            float distance = dx * dx + dy * dy + dz * dz;
            distance = sqrt(distance);

            if (distance <= radius) {

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

                terrainGenerationQueueMutex.lock();
                terrainGenerationQueue.push_back(&freshChunk->second);
                terrainGenerationQueueMutex.unlock();

              // if the chunk does exist, check if it it's loaded; if it is loaded then update its neighbors array and regenerate its mesh, otherwise just load it
              } else if (targetChunk->second.chunkIsLoaded() ) {
                // temporarily disabled because this nukes performance for some reason
                if (false && !chunkInPipeline(&targetChunk->second)) {
                  if( updateNeighbors(targetChunk->second) ) {
                    meshGenerationQueueMutex.lock();
                    meshGenerationQueue.push_back(&targetChunk->second);
                    meshGenerationQueueMutex.unlock();
                  }
                }
              } else if (!targetChunk->second.chunkIsLoaded() ) {
                if (!chunkInPipeline(&targetChunk->second)) {
                  meshGenerationQueueMutex.lock();
                  meshGenerationQueue.push_back(&targetChunk->second);
                  meshGenerationQueueMutex.unlock();
                }
              }
            }
          }
        }
      }
    }

    return;
  }

  // THREAD 2 - take an element from the terrain generation queue, generate it, and then send it to the mesh generation queue
  void processTerrainGenerationQueue(void) {
    while(shouldGenerateWorld) {
      if (!terrainGenerationQueue.empty()) {
        terrainGenerationQueue.front()->generateChunkState();

        meshGenerationQueueMutex.lock();
        meshGenerationQueue.push_back(terrainGenerationQueue.front());
        meshGenerationQueueMutex.unlock();

        terrainGenerationQueueMutex.lock();
        terrainGenerationQueue.pop_front();
        terrainGenerationQueueMutex.unlock();
      }
    }

    return;
  }

  // THREAD 3 - take an element from the mesh generation queue, generate it, and then send it to the gpu buffer queue
  void processMeshGenerationQueue(void) {
    while(shouldGenerateWorld) {
      if (!meshGenerationQueue.empty()) {
        // make sure that the chunk's terrain has been generated before attempting to generate a mesh
        // this step should be superfluous but for the sake of consistency with other pipeline steps i included it (plus the overhead is minimal)
        if (meshGenerationQueue.front()->terrainIsGenerated()) {
          meshGenerationQueue.front()->generateChunkMesh();

          bufferQueueMutex.lock();
          bufferQueue.push_back(meshGenerationQueue.front());
          bufferQueueMutex.unlock();

          meshGenerationQueueMutex.lock();
          meshGenerationQueue.pop_front();
          meshGenerationQueueMutex.unlock();
        }
      }
    }

    return;
  }

  // THREAD 4 - send chunks outside of render distance to the unload queue
  void unloadChunk() {
    while (shouldGenerateWorld) {
      // grab the current player/camera position
      float camX = camera.cameraPos.x;
      float camY = camera.cameraPos.y;
      float camZ = camera.cameraPos.z;

      // define a search radius around the current position
      int radius = renderDistance * 16;

      // iterate through every loaded chunk
      for (std::unordered_map<ChunkPos, Chunk>::iterator targetChunk = Chunks.begin(); targetChunk != Chunks.end(); targetChunk++) { 

        // verify that the chunk being checked is still loaded (don't add stuff to the queue twice)
        if (targetChunk->second.chunkIsLoaded()){

          // calculate the distance of the chunk away from the player
          float dx = camX - targetChunk->first.x * 16, dy = camY - targetChunk->first.y * 16, dz = camZ - targetChunk->first.z * 16;
          float distance = dx * dx + dy * dy + dz * dz;
          distance = std::sqrt(distance);

          // shrink the distance a little so there's a small buffer between loading and unloading (so border chunks don't flicker in and out)
          distance -= 20;

          // check if the chunk is outside of render distance, if so unload it
          if (distance > radius) {

            // make sure the chunk isn't already somewhere in the pipeline before trying to unload it
            if (!chunkInPipeline(&targetChunk->second)) {
              unloadQueueMutex.lock();
              unloadQueue.push_back(&targetChunk->second);
              unloadQueueMutex.unlock();
            }
          }
        }
      }
    }

    return;
  }

  // THREAD 0 (main) - take an element from the buffer queue and buffer its data to the gpu
  void processBufferQueue(void) {
    double startTime = glfwGetTime();
    double currentTime;

    // buffer chunks until 5 milliseconds passes (i don't check if the queue is empty because sometimes it is empty for a only a split
    // second and that causes a whole buffering job to be effectively canceled
    while (true) {

      if (!bufferQueue.empty()) {
        // ensure that a mesh has been generated for the chunk before sending it to the gpu
        if (bufferQueue.front()->meshIsReady()) {
          bufferQueue.front()->bufferChunkData();

          bufferQueueMutex.lock();
          bufferQueue.pop_front();
          bufferQueueMutex.unlock();
        }

      }

      // check how much time has passed and break if it has been over 5 milliseconds
      currentTime = glfwGetTime();
      if (currentTime - startTime > 0.005) {
        break;
      }

    }

    return;
  }

  // THREAD 0 (main) - unload all chunks outside of render distance
  void processUnloadQueue(void) {
    while (!unloadQueue.empty()) {
        unloadQueue.front()->unloadChunk();

        unloadQueueMutex.lock();
        unloadQueue.pop_front();
        unloadQueueMutex.unlock();
    }

    return;
  }

  // THREAD 0 (main) - loop through the unordered map and render it all
  void renderWorld(void) {
    for (std::unordered_map<ChunkPos, Chunk>::const_iterator it = Chunks.begin(); it != Chunks.end(); it++) { 
      if (it->second.chunkIsLoaded()) {
      // if the chunk mesh isn't ready, wait for a moment until the other thread finishes generating it
      // i'm actually not quite sure why this works since the chunk won't be buffered if it isn't already, but it works so i'm not touching it
      while (!it->second.meshIsReady()) {}

      // get the second element of the unordered map (so the chunk itself, not the hash)
      it->second.renderChunkMesh();
      }
    }

    return;
  }

  // return true if the target chunk is already in the rendering pipeline
  bool chunkInPipeline(const Chunk* CHUNK) {

    terrainGenerationQueueMutex.lock();
    std::deque<Chunk*>::const_iterator targetChunkTerrainQueuePosition = std::find(terrainGenerationQueue.begin(), terrainGenerationQueue.end(), CHUNK);
    bool inTerrainGenerationQueue = !(targetChunkTerrainQueuePosition == terrainGenerationQueue.end());
    terrainGenerationQueueMutex.unlock();
    if (inTerrainGenerationQueue) {
      return true;
    }

    meshGenerationQueueMutex.lock();
    std::deque<Chunk*>::const_iterator targetChunkMeshQueuePosition = std::find(meshGenerationQueue.begin(), meshGenerationQueue.end(), CHUNK);
    bool inMeshGenerationQueue = !(targetChunkMeshQueuePosition == meshGenerationQueue.end());
    meshGenerationQueueMutex.unlock();
    if (inMeshGenerationQueue) {
      return true;
    }

    bufferQueueMutex.lock();
    std::deque<Chunk*>::const_iterator targetChunkBufferQueuePosition = std::find(bufferQueue.begin(), bufferQueue.end(), CHUNK);
    bool inBufferQueue = !(targetChunkBufferQueuePosition == bufferQueue.end());
    bufferQueueMutex.unlock();
    if (inBufferQueue) {
      return true;
    }

    unloadQueueMutex.lock();
    std::deque<Chunk*>::const_iterator targetChunkUnloadQueuePosition = std::find(unloadQueue.begin(), unloadQueue.end(), CHUNK);
    bool inUnloadQueue = !(targetChunkUnloadQueuePosition == unloadQueue.end());
    unloadQueueMutex.unlock();
    if (inUnloadQueue) {
      return true;
    }

    // if every search turned up false, it should be safe to assume the chunk isn't in the pipeline
    return false;
  }

  // return a neighboring chunk in a specified direction if it exists
  Chunk* searchNeighbor(const Chunk& chunk, Direction direction) {
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
  bool updateNeighbors(Chunk& chunk) {

    bool chunkNeighorsChanged = false;

    // get the neighboring chunk, then check if it has already been set
    // if it has, then don't bother updating, if it hasn't, update and make note of the fact that you updated
    for (unsigned int i = NORTH; i <= BOTTOM; i++) {
      Chunk* newNeighbor = searchNeighbor(chunk, static_cast<Direction>(i)); 
      const Chunk* currentNeighbor = chunk.getNeighbor(static_cast<Direction>(i));
      if (newNeighbor != currentNeighbor ) {
        chunk.setNeighbor( newNeighbor, static_cast<Direction>(i) ); 
        chunkNeighorsChanged = true;
      }
    }

    return chunkNeighorsChanged;
  }

  void closeWorld(void) {
    shouldGenerateWorld = false;

    return;
  }

private:

  // store references to the camera and shader objects
  const Shader &customShader;
  const Camera &camera;

  // define how wide of an area around the player should be rendered
  const int renderDistance = 8;

  // atomic variable to control the stoppage of each independant thread
  std::atomic<bool> shouldGenerateWorld = true;

  // unordered map to store the data for every generated chunk
  std::unordered_map<ChunkPos, Chunk> Chunks; 

  // deque and mutex variables for each part of the pipeline
  // the deque stores the objects moving through the pipeline
  // the mutexes are used to make sure that no part of the pipeline is double accessed 
  std::deque<Chunk*> terrainGenerationQueue;
  std::mutex terrainGenerationQueueMutex;

  std::deque<Chunk*> meshGenerationQueue;
  std::mutex meshGenerationQueueMutex;

  std::deque<Chunk*> bufferQueue;
  std::mutex bufferQueueMutex;

  std::deque<Chunk*> unloadQueue;
  std::mutex unloadQueueMutex;
};
