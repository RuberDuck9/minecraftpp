#ifndef BLOCK_TYPES_H
#define BLOCK_TYPES_H

#include <stdint.h>

struct blockType {
  uint8_t blockID;
  uint8_t tu, tv;
  uint8_t su, sv;
  uint8_t bu, bv;
};

const blockType grass = { 
  1, 
  0, 0, 
  48, 0, 
  32, 0
};

const blockType stone = { 
  1, 
  16, 0, 
  16, 0, 
  16, 0
};

/*
const blockType grass = { 
  1, 
  0, 0, 
  48, 0, 
  32, 0
};
const blockType grass = { 
  1, 
  0, 0, 
  48, 0, 
  32, 0
};
const blockType grass = { 
  1, 
  0, 0, 
  48, 0, 
  32, 0
};
const blockType grass = { 
  1, 
  0, 0, 
  48, 0, 
  32, 0
};
const blockType grass = { 
  1, 
  0, 0, 
  48, 0, 
  32, 0
};

*/
#endif
