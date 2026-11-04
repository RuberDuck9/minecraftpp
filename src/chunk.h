// TODO: Rewrite chunk store logic to only use block ID's instead of the entire
// blockType struct
// TODO: implement the big three for the chunk class

#include <cstdint>
#include <stdint.h>
#include <vector>

#include "shader.h"
#include "block_types.h"

// define chunk dimensions
const inline uint8_t chunkSize = 16;

enum Direction { NORTH, SOUTH, EAST, WEST, TOP, BOTTOM };

struct ChunkPos {
  int64_t x, y, z;

  bool operator==(const ChunkPos& otherPos) const {
    return (x == otherPos.x 
         && y == otherPos.y 
         && z == otherPos.z);
  }
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
const inline FacePos faceCorners[6][4] = {
    {{0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}}, // north
    {{1, 0, 0}, {0, 0, 0}, {0, 1, 0}, {1, 1, 0}}, // south
    {{1, 0, 1}, {1, 0, 0}, {1, 1, 0}, {1, 1, 1}}, // east
    {{0, 0, 0}, {0, 0, 1}, {0, 1, 1}, {0, 1, 0}}, // west
    {{0, 1, 1}, {1, 1, 1}, {1, 1, 0}, {0, 1, 0}}, // top
    {{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}}, // bottom
};

class Chunk {

public:
  // constructor
  Chunk(const ChunkPos chunkPos, const Shader &customShader) : chunkPos(chunkPos), customShader(customShader) {
    model = glm::translate(glm::mat4(1.0f), glm::vec3(chunkPos.x * chunkSize, chunkPos.y * chunkSize, chunkPos.z * chunkSize));
    model = glm::scale(model, glm::vec3(255.0f, 255.0f, 255.0f));
    chunkInitialized = false;
    chunkGenerated = false;
    for (uint8_t x = 0; x < chunkSize; x++) {
      for (uint8_t y = 0; y < chunkSize; y++) {
         for (uint8_t z = 0; z < chunkSize; z++) {

          chunkState[x][y][z] = air;

        }
      }
    }
    VAO = 0;
    VBO = 0;
    EBO = 0;
  }

  // initialize the various array and buffer objects needed to render the chunk
  void initializeChunk(void);

  // assign a block ID to each position within the chunk
  void generateChunkState(void);

  // generate the actual chunk mesh
  void generateChunkMesh(void);

  // send the chunk mesh to the gpu
  void bufferChunkData(void) const;

  // render the loaded chunk gpu data
  void renderChunkMesh(void) const;

  // unload the chunk data from the gpu
  void unloadChunk(void);

  bool chunkIsInitialized() const;

  bool chunkIsGenerated() const;

  // function to return the position of the chunk
  ChunkPos getChunkPos(void) const;

private:
  // store the chunk coordinate (in increments of 16 compared to world
  // coordinates)
  const ChunkPos chunkPos;

  // array to store the unique block at each position within the chunk
  blockType chunkState[chunkSize][chunkSize][chunkSize];

  // vectors used to store the vertices and indices for the chunk mesh
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  // store a reference the shader in use (for uploading the model matrix to normalize coordinates during rendering)
  const Shader &customShader;

  // model matrix to store the chunk's position normalization
  glm::mat4 model;

  // vertex objects to store the chunk data on the gpu
  unsigned int VAO, VBO, EBO;

  // variables to store whether or not the chunk is loaded and generated
  bool chunkInitialized, chunkGenerated;

  // function to return the texture atlas coordinates for each block face
  UV getUV(const uint8_t &face, const blockType &block) const;

  // generate an individual chunk face and add it's data to the vertex and index
  // vectors
  void generateFace(const FacePos &facePos, const uint8_t &face, const blockType &block);
};
