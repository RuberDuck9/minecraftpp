#include "chunk.h"

// TODO: update the comments here

void Chunk::initializeChunk(void) {
  // generate the gpu objects for storing the chunk's data
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  // bind the vertex array object 
  glBindVertexArray(VAO);

  // bind each buffer object so buffer calls target them
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

  // tell opengl how to interpret the buffered vertex data
  glVertexAttribPointer(0, 3, GL_UNSIGNED_BYTE, GL_TRUE, 5 * sizeof(uint8_t), (void *)0);
  glEnableVertexAttribArray(0);

  // tell opengl how to interpret the texture data
  glVertexAttribPointer(1, 2, GL_UNSIGNED_BYTE, GL_FALSE, 5 * sizeof(uint8_t), (void *)(3 * sizeof(uint8_t)));
  glEnableVertexAttribArray(1);

  // unbind the vertex array objects (and subsequently the other buffer objects)
  glBindVertexArray(0);
}

void Chunk::generateChunkState(void) {

  for (uint8_t x = 0; x < chunkSize; x++) {
    for (uint8_t y = 0; y < chunkSize; y++) {
       for (uint8_t z = 0; z < chunkSize; z++) {

        if (y + chunkPos.y * chunkSize > 45) {
          chunkState[x][y][z] = air;
        } else if (y + chunkPos.y * chunkSize > 44) {
          chunkState[x][y][z] = grass;
        } else if (y + chunkPos.y * chunkSize > 40) {
          chunkState[x][y][z] = dirt;
        } else if (y + chunkPos.y * chunkSize == 0) {
          chunkState[x][y][z] = bedrock;
        } else {
          chunkState[x][y][z] = stone;
        }

      }
    }
  }

  return;
}

void Chunk::generateChunkMesh(void) {

  for (uint8_t x = 0; x < chunkSize; x++) {
    for (uint8_t y = 0; y < chunkSize; y++) {
       for (uint8_t z = 0; z < chunkSize; z++) {

        if (chunkState[x][y][z].blockID != 0) {
          generateFace({x, y, z}, 0, chunkState[x][y][z]);
          generateFace({x, y, z}, 1, chunkState[x][y][z]);
          generateFace({x, y, z}, 2, chunkState[x][y][z]);
          generateFace({x, y, z}, 3, chunkState[x][y][z]);
          generateFace({x, y, z}, 4, chunkState[x][y][z]);
          generateFace({x, y, z}, 5, chunkState[x][y][z]);
        }

      }
    }
  }

  return;
}

void Chunk::bufferChunkData(void) const {
  // bind the vertex array object 
  glBindVertexArray(VAO);

  // bind each buffer object so buffer calls target them
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

  // buffer the vertex and index data to the gpu objects just made
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

  glBindVertexArray(0);
}

void Chunk::renderChunkMesh(void) const {
  // bind the vertex array object 
  glBindVertexArray(VAO);


  // define the model matrix to scale up the chunk and normalize it"s coordinates
  glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(chunkPos.x * chunkSize, chunkPos.y * chunkSize, chunkPos.z * chunkSize));
  model = glm::scale(model, glm::vec3(255.0f, 255.0f, 255.0f));
  customShader.setMat4("model", model);

  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

  // unbind the vertex array objects (and subsequently the other buffer objects)
  glBindVertexArray(0);

  return;
}

UV Chunk::getUV(const uint8_t &face, const blockType &block) const {
  if (face < 4) {
    return {block.su, block.sv};   
  } else if (face < 5) {
    return {block.tu, block.tv};   
  } else {
    return {block.bu, block.bv};   
  }
} 

void Chunk::generateFace(const FacePos &facePos, const uint8_t &face, const blockType &block) {
  unsigned int index = vertices.size();

  UV uv = getUV(face, block);

  // Textures coordinates are assigned 'upside down' here depending on how you look at it
  // This is done intentionally because the texture atlas itself is loaded upside down to ensure that 0 0 refers to the top left
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
