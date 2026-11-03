// TODO: implement rendering queue

#include <functional>
#include <unordered_map>

#include "chunk.h"

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
  World(const Shader &customShader) : customShader(customShader) {}

  // generate a new chunk, load it, and add it to the unordered map
  void generateChunk(const ChunkPos newChunkPos) {
    Chunk newChunk(newChunkPos, customShader);
    newChunk.initializeChunk();
    newChunk.generateChunkState();
    newChunk.generateChunkMesh();
    newChunk.bufferChunkData();

    loadedChunks.insert({newChunkPos, newChunk});
  }

  // loop through the unordered map and render it all
  void renderWorld(void) {
    for (std::unordered_map<ChunkPos, Chunk>::iterator it = loadedChunks.begin(); it != loadedChunks.end(); it++) { 
      // get the second element of the unordered map (so the chunk itself, not the hash)
      it->second.renderChunkMesh();
    }
  }


private:
  const Shader &customShader;

  std::unordered_map<ChunkPos, Chunk> loadedChunks; 
};
