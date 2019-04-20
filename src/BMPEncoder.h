#ifndef BMPEncoderH
#define BMPEncoderH

#include "BMPCommon.h"

class BMPEncoder
{
public:
  BMPEncoder();

  bool encode(const BmpData & bmpData, VecByte & encoded);
};

#endif // BMPEncoderH
