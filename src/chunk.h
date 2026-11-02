#include <vector>
#include <stdint.h>

#include "shader.h"

#include "block_types.h"

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

inline FacePos faceCorners[6][4] = {
        { {0,0,1}, {1,0,1}, {1,1,1}, {0,1,1} }, // North
        { {1,0,0}, {0,0,0}, {0,1,0}, {1,1,0} }, // South
        { {1,0,1}, {1,0,0}, {1,1,0}, {1,1,1} }, // East
        { {0,0,0}, {0,0,1}, {0,1,1}, {0,1,0} }, // West
        { {0,1,1}, {1,1,1}, {1,1,0}, {0,1,0} }, // Top
        { {0,0,0}, {1,0,0}, {1,0,1}, {0,0,1} }, // Bottom
};

class chunk {
 
  public:
    chunk(ChunkPos chunkPos, const Shader& customShader, uint8_t chunkSize) : chunkPos(chunkPos), customShader(customShader), chunkSize(chunkSize) {}

    UV getUV(uint8_t face, blockType block) {
      if (face < 4) {
        return {block.su, block.sv};   
      } else if (face < 5) {
        return {block.tu, block.tv};   
      } else {
        return {block.bu, block.bv};   
      }
    } 

    // TODO: Rewite like half this codebase

    void generateFace(FacePos facePos, uint8_t face, blockType block) {
      unsigned int index = vertices.size();

      UV uv = getUV(face, block);

      // Textures coordinates are assigned 'upside down' here depending on how you look at it
      // This is done intentionally because the texture atlas is self is loaded upside down to ensure that 0 0 refers to the top left
      vertices.push_back({static_cast<uint8_t>(faceCorners[face][0].x + facePos.x), 
                          static_cast<uint8_t>(faceCorners[face][0].y + facePos.y), 
                          static_cast<uint8_t>(faceCorners[face][0].z + facePos.z), 
                          static_cast<uint8_t>(uv.u + 15), 
                          static_cast<uint8_t>(uv.v + 15)}); 
      vertices.push_back({static_cast<uint8_t>(faceCorners[face][1].x + facePos.x), 
                          static_cast<uint8_t>(faceCorners[face][1].y + facePos.y), 
                          static_cast<uint8_t>(faceCorners[face][1].z + facePos.z), 
                          static_cast<uint8_t>(uv.u +  0), 
                          static_cast<uint8_t>(uv.v +  15)}); 
      vertices.push_back({static_cast<uint8_t>(faceCorners[face][2].x + facePos.x), 
                          static_cast<uint8_t>(faceCorners[face][2].y + facePos.y), 
                          static_cast<uint8_t>(faceCorners[face][2].z + facePos.z), 
                          static_cast<uint8_t>(uv.u +  0), 
                          static_cast<uint8_t>(uv.v +  0)}); 
      vertices.push_back({static_cast<uint8_t>(faceCorners[face][3].x + facePos.x), 
                          static_cast<uint8_t>(faceCorners[face][3].y + facePos.y), 
                          static_cast<uint8_t>(faceCorners[face][3].z + facePos.z), 
                          static_cast<uint8_t>(uv.u + 15), 
                          static_cast<uint8_t>(uv.v +  0)}); 

      indices.push_back(index + 0);
      indices.push_back(index + 1);
      indices.push_back(index + 2);

      indices.push_back(index + 2);
      indices.push_back(index + 3);
      indices.push_back(index + 0);

      return;
    }

    void generateChunkMesh(void) {

      for (uint8_t x = 0; x < chunkSize; x++) {
        for (uint8_t y = 0; y < chunkSize; y++) {
           for (uint8_t z = 0; z < chunkSize; z++) {

            generateFace({x, y, z}, 0, grass);
            generateFace({x, y, z}, 1, grass);
            generateFace({x, y, z}, 2, grass);
            generateFace({x, y, z}, 3, grass);
            generateFace({x, y, z}, 4, grass);
            generateFace({x, y, z}, 5, grass);

          }
        }
      }

      glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(chunkPos.x * chunkSize, chunkPos.y * chunkSize, chunkPos.z * chunkSize));
      model = glm::scale(model, glm::vec3(255.0f, 255.0f, 255.0f));
      customShader.setMat4("model", model);

      // buffer the vertex and index data
      glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

      return;
    }

    void renderChunkMesh(void) const {
      glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
      return;
    }

  private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    ChunkPos chunkPos;
    const Shader& customShader;
    const uint8_t chunkSize;
 
};
