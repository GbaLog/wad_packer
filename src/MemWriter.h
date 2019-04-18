#ifndef MEMWRITER_H
#define MEMWRITER_H

#include <cstdint>

class MemWriter
{
public:
  MemWriter(uint8_t * data, size_t size);

  bool writeUint8(uint8_t val);
  bool writeUint16(uint16_t val);
  bool writeUint32(uint32_t val);
  bool writeUint64(uint64_t val);
  bool writeData(const void * data, size_t size);

  bool seek(size_t pos);
  bool shift(int64_t val);
  bool shiftFromStart(uint32_t val);

  uint32_t getOffset() const;
  uint32_t getRemainingSize() const;
  const uint8_t * getPos() const;

private:
  uint8_t * _data;
  const size_t _size;
  uint8_t * _pos;
};

#endif // MEMWRITER_H
