// TODO: Rewrite chunk store logic to only use block ID's instead of the entire blockType struct
// TODO: implement the big three for the chunk class

#include <cstdint>
#include <vector>
#include <stdint.h>

#include "shader.h"

#include "block_types.h"

const uint8_t chunkSize = 16;

struct ChunkPos {
  int64_t x, y, z;
};

struct FacePos {
  uint8_t x, y, z;
};

struct Vertex {
  uint8_t x, y, z;
  uint8_t u, v;
};

struct UV {
  uint8_t u, v;
};

// array to assist with sorting out each individual vertex for each voxel face
inline FacePos faceCorners[6][4] = {
        { {0,0,1}, {1,0,1}, {1,1,1}, {0,1,1} }, // North
        { {1,0,0}, {0,0,0}, {0,1,0}, {1,1,0} }, // South
        { {1,0,1}, {1,0,0}, {1,1,0}, {1,1,1} }, // East
        { {0,0,0}, {0,0,1}, {0,1,1}, {0,1,0} }, // West
        { {0,1,1}, {1,1,1}, {1,1,0}, {0,1,0} }, // Top
        { {0,0,0}, {1,0,0}, {1,0,1}, {0,0,1} }, // Bottom
};

class Chunk {
 
  public:
    Chunk(const ChunkPos chunkPos, const Shader& customShader) : chunkPos(chunkPos), customShader(customShader) {}

    // initialize the various array and buffer objects needed to render the chunk
    void initializeChunk(void);

    // assign a block ID to each position within the chunk
    void generateChunkState(void);

    // generate the actualy chunk mesh
    void generateChunkMesh(void);

    // send the chunk mesh to the gpu
    void bufferChunkData(void) const;

    // render the loaded chunk gpu data
    void renderChunkMesh(void) const;

  private:
    
    // store the chunk coordinate (in increments of 16 compared to world coordinates)
    const ChunkPos chunkPos;

    // array to store the unique block at each position within the chunk
    blockType chunkState[chunkSize][chunkSize][chunkSize];

    // vectors used to store the vertices and indices for the chunk mesh
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // store a reference the shader in use (for uploading the model matrix to normalize coordinates during rendering)
    const Shader& customShader;

    // vertex objects to store the chunk data on the gpu
    unsigned int VAO, VBO, EBO;

    // function to return the texture atlas coordinates for each block face
    UV getUV(const uint8_t &face, const blockType &block) const;

    // generate an individual chunk face and add it's data to the vertex and index vectors
    void generateFace(const FacePos &facePos, const uint8_t &face, const blockType &block);

};
