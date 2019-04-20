#include "WadDecoder.h"

bool isWadFormat(uint32_t magic)
{
  const uint32_t wadFormat = (0x00 << 24) | ('D' << 16) | ('A' << 8) | 'W';
  return (magic & 0x00FFFFFF) == wadFormat;
}


WadDecoder::WadDecoder()
{

}

TextureItem::TextureItem(MemReader reader)
{
  if (reader.readData((uint8_t*)&_params, sizeof(_params)) == false)
  {
    TRACE(ERR) << "Can't read texture params";
    throw std::runtime_error("Can't read texture params");
  }

  _image.resize(_params._width * _params._height);

  if (reader.seek(_params._offsetOfImage) == false)
  {
    TRACE(ERR) << "Can't shift to image";
    throw std::runtime_error("Can't shift to image");
  }

  uint32_t w = _params._width;
  uint32_t h = _params._height;

  reader.readData(_image.data(), _image.size());
  _mipmap1.resize((w / 2) * (h / 2));
  reader.readData(_mipmap1.data(), _mipmap1.size());
  _mipmap2.resize((w / 4) * (h / 4));
  reader.readData(_mipmap2.data(), _mipmap2.size());
  _mipmap3.resize((w / 8) * (h / 8));
  reader.readData(_mipmap3.data(), _mipmap3.size());

  // Mandatory 2 bytes
  if (reader.shift(2) == false)
  {
    TRACE(ERR) << "Can't shift to RGB map";
    throw std::runtime_error("Can't shift to RGB map");
  }

  if (reader.getRemainingSize() != 256 * 3 + 2)
  {
    TRACE(ERR) << "Can't load color palette, remaining size: " << reader.getRemainingSize();
    throw std::runtime_error("Can't load color palette");
  }

  _redColor.resize(256);
  _greenColor.resize(256);
  _blueColor.resize(256);
  for (int i = 0; i < 256; ++i)
  {
    uint8_t & colorRed   = _redColor[i];
    uint8_t & colorGreen = _greenColor[i];
    uint8_t & colorBlue  = _blueColor[i];
    reader.readUint8(colorRed);
    reader.readUint8(colorGreen);
    reader.readUint8(colorBlue);
  }
}
