#ifndef BLOCK_TYPES_H
#define BLOCK_TYPES_H

#include <stdint.h>

struct blockType {
  uint8_t blockID;
  uint8_t tu, tv;
  uint8_t su, sv;
  uint8_t bu, bv;

  bool operator==(const blockType& OTHERBLOCK) const {
    return blockID == OTHERBLOCK.blockID;
  }
};

const blockType air = { 
  0, 
  0, 160, 
  0, 160, 
  0, 160
};

const blockType grass = { 
  1, 
  0, 0, 
  48, 0, 
  32, 0
};

const blockType stone = { 
  3, 
  16, 0, 
  16, 0, 
  16, 0
};

const blockType dirt = { 
  1, 
  32, 0, 
  32, 0, 
  32, 0
};

const blockType wood = { 
  1, 
  64, 0, 
  64, 0, 
  64, 0
};

const blockType bedrock = { 
  1, 
  16, 16, 
  16, 16, 
  16, 16
};

#endif
