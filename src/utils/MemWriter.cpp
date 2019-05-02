#include "MemWriter.h"
#include <cstring>
//-----------------------------------------------------------------------------
MemWriter::MemWriter(uint8_t * data, size_t size) :
  _data(data),
  _size(size),
  _pos(_data)
{}
//-----------------------------------------------------------------------------
bool MemWriter::writeUint8(uint8_t val)
{
  return writeData(&val, 1);
}
//-----------------------------------------------------------------------------
bool MemWriter::writeUint16(uint16_t val)
{
  return writeData(&val, 2);
}
//-----------------------------------------------------------------------------
bool MemWriter::writeUint32(uint32_t val)
{
  return writeData(&val, 4);
}
//-----------------------------------------------------------------------------
bool MemWriter::writeUint64(uint64_t val)
{
  return writeData(&val, 8);
}
//-----------------------------------------------------------------------------
bool MemWriter::writeData(const void * data, size_t size)
{
  if ((_pos - _data) + size > _size)
    return false;

  memcpy(_pos, data, size);
  _pos += size;
  return true;
}
//-----------------------------------------------------------------------------
bool MemWriter::shiftFromStart(uint32_t val)
{
  if (val >= _size)
    return false;

  _pos = _data + val;
  return true;
}
//-----------------------------------------------------------------------------
uint32_t MemWriter::getRemainingSize() const
{
  return _size - (_pos - _data);
}
//-----------------------------------------------------------------------------
uint8_t * MemWriter::getPos()
{
  return _pos;
}
//-----------------------------------------------------------------------------
