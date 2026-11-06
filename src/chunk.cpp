#include "chunk.h"
#include "shader.h"

// *************** PUBLIC STUFF ******************

// constructor
Chunk::Chunk(const ChunkPos chunkPos, const Shader &customShader) : chunkPos(chunkPos), customShader(customShader) {

  // definte the model matrix
  model = glm::translate(glm::mat4(1.0f), glm::vec3(chunkPos.x * chunkSize, chunkPos.y * chunkSize, chunkPos.z * chunkSize));
  model = glm::scale(model, glm::vec3(255.0f, 255.0f, 255.0f));

  // initialize the state variables
  chunkInitialized = false;
  terrainGenerated = false;
  meshReady = false;
  chunkLoaded = false;

  // initialize the gpu object IDs to 0
  VAO = 0;
  VBO = 0;
  EBO = 0;

  // give the chunk terrain default values
  for (uint8_t x = 0; x < chunkSize; x++) {
    for (uint8_t y = 0; y < chunkSize; y++) {
       for (uint8_t z = 0; z < chunkSize; z++) {

        chunkState[x][y][z] = air;

      }
    }
  }

  // assign nullptr's to all of the neighbors by default
  for (unsigned int i = NORTH; i <= BOTTOM; i++) {
    neighbors[static_cast<Direction>(i)] = nullptr;
  }
}

// assign a block ID to each position within the chunk
void Chunk::generateChunkState(void) {

  // make sure that the chunk isn't loaded since its data is about to be deleted
  terrainGenerated = false;
  meshReady = false;

  for (uint8_t x = 0; x < chunkSize; x++) {
    for (uint8_t y = 0; y < chunkSize; y++) {
       for (uint8_t z = 0; z < chunkSize; z++) {

        if (y + chunkPos.y * chunkSize > 45) {
          chunkState[x][y][z] = air;
        } else if (y + chunkPos.y * chunkSize > 44) {
          chunkState[x][y][z] = grass;
        } else if (y + chunkPos.y * chunkSize > 40) {
          chunkState[x][y][z] = dirt;
        } else {
          chunkState[x][y][z] = stone;
        }

      }
    }
  }

  // mark the chunk as generated so it can progress through the pipeline
  terrainGenerated = true;

  return;
}

// generate the actual chunk mesh
void Chunk::generateChunkMesh(void) {

  // make sure that the chunk isn't loaded since its data is about to be deleted
  meshReady = false;

  // remove any existing mesh data before generating new stuff
  vertices.clear();
  indices.clear();

  for (uint8_t x = 0; x < chunkSize; x++) {
    for (uint8_t y = 0; y < chunkSize; y++) {
       for (uint8_t z = 0; z < chunkSize; z++) {

        if (chunkState[x][y][z].blockID != air.blockID) {
          for (unsigned int i = NORTH; i <= BOTTOM; i++) {
            if (faceExposed({x, y, z}, static_cast<Direction>(i))) {
              generateFace({x, y, z}, static_cast<Direction>(i), chunkState[x][y][z]);
            }
          }
        }

      }
    }
  }
  // mark the chunk as ready to be loaded
  meshReady = true;

  return;
}

// send the chunk mesh to the gpu
void Chunk::bufferChunkData(void) {

  // NOTE *********************
  // Ideally, the next 25 lines of code would only run on the first buffer of a chunk and would be cached afterwards,
  // however, for some reason when I do that it doesn't work (well it sorta does but the results are interesting to
  // say the least) SO, for now I just have it run every time since it makes stuff work and the performance overhead
  // is relatively minimal (hopefully)

  // generate the vertex array object
  glGenVertexArrays(1, &VAO);

  // bind the vertex array object 
  glBindVertexArray(VAO);

  // generate the gpu objects for storing the chunk's data
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  // bind each buffer objects so config calls target them
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

  // tell opengl how to interpret the buffered vertex data
  glVertexAttribPointer(0, 3, GL_UNSIGNED_BYTE, GL_TRUE, 5 * sizeof(uint8_t), (void *)0);
  glEnableVertexAttribArray(0);

  // tell opengl how to interpret the texture data
  glVertexAttribPointer(1, 2, GL_UNSIGNED_BYTE, GL_FALSE, 5 * sizeof(uint8_t), (void *)(3 * sizeof(uint8_t)));
  glEnableVertexAttribArray(1);

  // unbind the vertex array object
  glBindVertexArray(0);

  // bind the vertex array object 
  glBindVertexArray(VAO);

  // bind each buffer object so buffer calls target them
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

  // buffer the vertex and index data to the gpu objects just made
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

  chunkLoaded = true;

  // unbind the vertex objects
  glBindVertexArray(0);

  return;
}

// render the loaded chunk gpu data
void Chunk::renderChunkMesh(void) const {

  // bind the vertex array object 
  glBindVertexArray(VAO);

  // load the model matrix onto the gpu
  customShader.setMat4("model", model);

  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

  // unbind the vertex array objects (and subsequently the other buffer objects)
  glBindVertexArray(0);

  return;
}

void Chunk::unloadChunk(void) {

  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
  glDeleteBuffers(1, &VAO);

  chunkInitialized = false;
  meshReady = false;

  chunkLoaded = false;

  return;
}

// assign a pointer to a neighboring chunk in the neighbors array
void Chunk::setNeighbor(Chunk* chunk, Direction direction) {
  neighbors[direction] = chunk;
  return;
}

// return a neighboring chunk in a specified direction if it exists
const Chunk* Chunk::getNeighbor(Direction direction) const {
  // generate a position for where to search based on the current chunk position and the search direction
  switch (direction) {
    case NORTH:
      return neighbors[NORTH]; 
      break;
    case SOUTH:
      return neighbors[SOUTH]; 
      break;
    case EAST:
      return neighbors[EAST]; 
      break;
    case WEST:
      return neighbors[WEST]; 
      break;
    case ABOVE:
      return neighbors[ABOVE]; 
      break;
    case BELOW:
      return neighbors[BELOW]; 
      break;
    // if an invald enum is passed, return a nullptr since this is indefined behavior (and should never happen)
    default:
      return nullptr;
  }
}

// function to return the position of the chunk
ChunkPos Chunk::getChunkPos(void) const {
  return this->chunkPos;
}

// return if the chunk's terrain has been generated
bool Chunk::terrainIsGenerated(void) const {
    return this->terrainGenerated;
}

// return if the chunk is initialized
bool Chunk::meshIsReady(void) const {
  return this->meshReady;
}

// return if the chunk is loaded (or should be anyway)
bool Chunk::chunkIsLoaded(void) const {
  return this->chunkLoaded;
}

// *************** PRIVATE STUFF ******************

// return true if a face neighbors air in a given direction
bool Chunk::faceExposed(const FacePos &FACEPOS, Direction direction) const {

  // separate out each direction to make the following code cleaner
  uint8_t faceX = FACEPOS.x, faceY = FACEPOS.y, faceZ = FACEPOS.z;

  // for each direction, check if it is on a chunk border, if so check the neighboring chunk's block in that direction for air
  // if it is not on a chunk border, just check this chunk's internal state for if it is air or not
  switch(direction){
    case NORTH:
      if (faceZ == chunkSize - 1) {
        return true;
      } else {
        return chunkState[faceX][faceY][faceZ + 1] == air;
      }
    case SOUTH:
      if (faceZ == 0) {
        return true;
      } else {
        return chunkState[faceX][faceY][faceZ - 1] == air;
      }
  case EAST:
      if (faceX == chunkSize - 1) {
        return true;
      } else {
        return chunkState[faceX + 1][faceY][faceZ] == air;
      }
  case WEST:
      if (faceX == 0) {
        return true;
      } else {
        return chunkState[faceX - 1][faceY][faceZ] == air;
      }
  case TOP:
      if (faceY == chunkSize - 1) {
        return true;
      } else {
        return chunkState[faceX][faceY + 1][faceZ] == air;
      }
  case BOTTOM:
      if (faceY == 0) {
        return true;
      } else {
        return chunkState[faceX][faceY - 1][faceZ] == air;
      }
    // if some invalid enum is passed, render just to be safe
    default:
        return true;
  }
}

// function to return the texture atlas coordinates for each block face
UV Chunk::getUV(const uint8_t &face, const blockType &block) const {
  if (face < 4) {
    return {block.su, block.sv};   
  } else if (face < 5) {
    return {block.tu, block.tv};   
  } else {
    return {block.bu, block.bv};   
  }
} 

// generate an individual chunk face and add it's data to the vertex and index vectors
void Chunk::generateFace(const FacePos &facePos, const uint8_t &face, const blockType &block) {
  unsigned int index = vertices.size();

  UV uv = getUV(face, block);

  // textures coordinates are assigned upside down here depending on how you look at it
  // this is done intentionally because the texture atlas itself is loaded upside down to ensure that 0 0 refers to the top left
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


