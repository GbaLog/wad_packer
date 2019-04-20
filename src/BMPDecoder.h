#ifndef BMPDECODER_H
#define BMPDECODER_H

#include "BMPCommon.h"

class BMPDecoder
{
public:
  BMPDecoder();
  bool decode(const VecByte & bmpBytes, BmpData & data);
};

#endif // BMPDECODER_H
