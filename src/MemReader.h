#ifndef MemReaderH
#define MemReaderH

#include <cstdint>

class MemReader
{
public:
  MemReader(const uint8_t * data, size_t size);

  bool readUint8(uint8_t & val);
  bool readUint16(uint16_t & val);
  bool readUint32(uint32_t & val);
  bool readUint64(uint64_t & val);
  bool readData(uint8_t * data, size_t size);

  bool seek(size_t pos);
  bool shift(int64_t val);
  bool shiftFromStart(uint32_t val);

  uint32_t getOffset() const;
  uint32_t getRemainingSize() const;
  const uint8_t * getPos() const;

private:
  const uint8_t * _data;
  size_t _size;
  const uint8_t * _pos;
};

#endif // MemReaderH
