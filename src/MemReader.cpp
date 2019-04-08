#include "MemReader.h"

MemReader::MemReader(const uint8_t * data, size_t size) :
  _data(data),
  _size(size),
  _pos(_data)
{}

bool MemReader::readUint8(uint8_t & val)
{
  return readData((uint8_t *)&val, sizeof(val));
}

bool MemReader::readUint16(uint16_t & val)
{
  return readData((uint8_t *)&val, sizeof(val));
}

bool MemReader::readUint32(uint32_t & val)
{
  return readData((uint8_t *)&val, sizeof(val));
}

bool MemReader::readUint64(uint64_t & val)
{
  return readData((uint8_t *)&val, sizeof(val));
}

bool MemReader::readData(uint8_t * data, size_t size)
{
  if ((_pos - _data) + size >= _size)
    return false;

  while (size) { *data++ = *_pos++; --size; }
  return true;
}

bool MemReader::seek(size_t pos)
{
  if (pos >= _size)
    return false;

  _pos = _data + pos;
}

bool MemReader::shift(int64_t val)
{
  if (_pos - _data + val >= _size)
    return false;

  _pos += val;
  return true;
}
