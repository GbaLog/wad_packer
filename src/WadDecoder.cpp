#include "WadDecoder.h"
//-----------------------------------------------------------------------------
bool isWadFormat(uint32_t magic)
{
  const uint32_t wadFormat = (0x00 << 24) | ('D' << 16) | ('A' << 8) | 'W';
  return (magic & 0x00FFFFFF) == wadFormat;
}
//-----------------------------------------------------------------------------
uint32_t getMipmap1Size(uint32_t origWidth, uint32_t origHeight)
{
  return (origWidth / 2) * (origHeight / 2);
}
//-----------------------------------------------------------------------------
uint32_t getMipmap2Size(uint32_t origWidth, uint32_t origHeight)
{
  return (origWidth / 4) * (origHeight / 4);
}
//-----------------------------------------------------------------------------
uint32_t getMipmap3Size(uint32_t origWidth, uint32_t origHeight)
{
  return (origWidth / 8) * (origHeight / 8);
}
//-----------------------------------------------------------------------------
bool WadDecoder::decode(const VecByte & buffer, WadFileData & wadData) const
{
  MemReader reader(buffer.data(), buffer.size());

  WadHeader wh;
  reader.readData((uint8_t*)&wh, sizeof(WadHeader));

  if (isWadFormat(wh._magicWord) == false)
  {
    TRACE(ERR) << "It's not WAD format: " << std::string((char*)(&wh._magicWord), 4);
    return false;
  }

  TRACE(DBG) << "WAD header: number of all textures: " << wh._numOfTextures << ", offset: " << wh._offsetOfLumps;

  reader.shiftFromStart(wh._offsetOfLumps);

  //Load all lumps first
  wadData._textures.reserve(wh._numOfTextures);
  for (uint32_t i = 0; i < wh._numOfTextures; ++i)
  {
    WadTexture texture;
    if (reader.readData((uint8_t*)&texture._lump, sizeof(WadLumpHeader)) == false)
    {
      TRACE(WRN) << "Can't read lump info: " << i + 1;
      continue;
    }

    if (texture._lump._type != 0x43)
    {
      TRACE(WRN) << "Type of texture is not 0x43. Only 0x43 supported: type: " << texture._lump._type;
      continue;
    }

    wadData._textures.push_back(texture);
  }

  for (auto & texture : wadData._textures)
  {
    WadLumpHeader & lump = texture._lump;
    reader.shiftFromStart(lump._offsetOfTexture);

    MemReader textureReader = MemReader(reader.getPos(), lump._fullLen);
    if (decodeTextureBody(textureReader, texture._header, texture._body) == false)
    {
      TRACE(ERR) << "Can't decode texture body: " << lump._name;
      return false;
    }
  }
  return true;
}
//-----------------------------------------------------------------------------
bool WadDecoder::decodeTextureBody(MemReader reader, WadTextureHeader & header, WadTextureBody & body) const
{
  if (reader.readData((uint8_t*)&header, sizeof(header)) == false)
  {
    TRACE(ERR) << "Can't read texture params";
    return false;
  }

  uint32_t w = header._width;
  uint32_t h = header._height;

  if (reader.shiftFromStart(header._offsetOfImage) == false)
  {
    TRACE(ERR) << "Can't shift to image";
    return false;
  }

  body._image.resize(header._width * header._height);
  body._mipmap1.resize((w / 2) * (h / 2));
  body._mipmap2.resize((w / 4) * (h / 4));
  body._mipmap3.resize((w / 8) * (h / 8));

  if (reader.readData(body._image.data(), body._image.size()) == false)
  {
    TRACE(ERR) << "Can't read original image: " << header._name;
    return false;
  }

  if (reader.readData(body._mipmap1.data(), body._mipmap1.size()) == false)
  {
    TRACE(ERR) << "Can't read mipmap 1 of image: " << header._name;
    return false;
  }

  if (reader.readData(body._mipmap2.data(), body._mipmap2.size()) == false)
  {
    TRACE(ERR) << "Can't read mipmap 2 of image: " << header._name;
    return false;
  }

  if (reader.readData(body._mipmap3.data(), body._mipmap3.size()) == false)
  {
    TRACE(ERR) << "Can't read mipmap 3 of image: " << header._name;
    return false;
  }

  // Mandatory 2 bytes 0x00, 0x01
  if (reader.shift(2) == false)
  {
    TRACE(ERR) << "Can't shift to RGB map";
    return false;
  }

  //Full palette must be 768 bytes + 2 padding bytes
  if (reader.getRemainingSize() != 256 * 3 + 2)
  {
    TRACE(ERR) << "Can't load color palette, remaining size: " << reader.getRemainingSize();
    return false;
  }

  body._colors.resize(256);
  for (int i = 0; i < 256; ++i)
  {
    if (reader.readData((uint8_t *)&body._colors[i], sizeof(WadTextureColor)) == false)
    {
      TRACE(ERR) << "Can't read color with position: " << i + 1;
      return false;
    }
  }

  return true;
}
//-----------------------------------------------------------------------------
