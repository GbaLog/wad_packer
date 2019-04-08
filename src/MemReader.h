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

private:
  const uint8_t * _data;
  size_t _size;
  const uint8_t * _pos;
};

#endif // MemReaderH
