#include "MemReader.h"
#include <cstring>
//-----------------------------------------------------------------------------
MemReader::MemReader(const uint8_t * data, size_t size) :
  _data(data),
  _size(size),
  _pos(_data)
{}
//-----------------------------------------------------------------------------
bool MemReader::readUint8(uint8_t & val)
{
  return readData((uint8_t *)&val, sizeof(val));
}
//-----------------------------------------------------------------------------
bool MemReader::readUint16(uint16_t & val)
{
  return readData((uint8_t *)&val, sizeof(val));
}
//-----------------------------------------------------------------------------
bool MemReader::readUint32(uint32_t & val)
{
  return readData((uint8_t *)&val, sizeof(val));
}
//-----------------------------------------------------------------------------
bool MemReader::readUint64(uint64_t & val)
{
  return readData((uint8_t *)&val, sizeof(val));
}
//-----------------------------------------------------------------------------
bool MemReader::readData(uint8_t * data, size_t size)
{
  if ((_pos - _data) + size > _size)
    return false;

  memcpy(data, _pos, size);
  _pos += size;
  return true;
}
//-----------------------------------------------------------------------------
bool MemReader::shift(int64_t val)
{
  if (_pos - _data + val >= _size)
    return false;

  _pos += val;
  return true;
}
//-----------------------------------------------------------------------------
bool MemReader::shiftFromStart(uint32_t val)
{
  if (val >= _size)
    return false;

  _pos = _data + val;
  return true;
}
//-----------------------------------------------------------------------------
uint32_t MemReader::getOffset() const
{
  return _pos - _data;
}
//-----------------------------------------------------------------------------
uint32_t MemReader::getRemainingSize() const
{
  return _size - (_pos - _data);
}
//-----------------------------------------------------------------------------
const uint8_t * MemReader::getPos() const
{
  return _pos;
}
//-----------------------------------------------------------------------------
