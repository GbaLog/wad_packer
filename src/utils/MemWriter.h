#ifndef RatelMemWriterH
#define RatelMemWriterH
//-----------------------------------------------------------------------------
#include <cstdint>
//-----------------------------------------------------------------------------
class MemWriter
{
public:
  MemWriter(uint8_t * data, size_t size);
  MemWriter(const MemWriter & rhs) = default;
  MemWriter & operator =(const MemWriter & rhs) = default;

  bool writeUint8(uint8_t val);
  bool writeUint16(uint16_t val);
  bool writeUint32(uint32_t val);
  bool writeUint64(uint64_t val);
  bool writeData(const void * data, size_t size);

  bool shift(int64_t val);
  bool shiftFromStart(uint32_t val);

  uint32_t getRemainingSize() const;
  const uint8_t * getPos() const;
  uint8_t * getPos();

private:
  uint8_t * _data;
  size_t _size;
  uint8_t * _pos;
};
//-----------------------------------------------------------------------------
#endif // RatelMemWriterH
//-----------------------------------------------------------------------------
