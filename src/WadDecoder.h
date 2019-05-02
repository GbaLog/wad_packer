#ifndef WadDecoderH
#define WadDecoderH
//-----------------------------------------------------------------------------
#include <cstdint>
#include "Tracer.h"
#include "MemReader.h"
#include "InetUtils.h"
#include "Common.h"
//-----------------------------------------------------------------------------
bool isWadFormat(uint32_t magic);
uint32_t getMipmap1Size(uint32_t origWidth, uint32_t origHeight);
uint32_t getMipmap2Size(uint32_t origWidth, uint32_t origHeight);
uint32_t getMipmap3Size(uint32_t origWidth, uint32_t origHeight);
//-----------------------------------------------------------------------------
STRUCT_PACK_BEGIN
struct WadHeader
{
  uint32_t _magicWord;
  uint32_t _numOfTextures;
  uint32_t _offsetOfLumps;
};
STRUCT_PACK_END
//-----------------------------------------------------------------------------
STRUCT_PACK_BEGIN
struct WadLumpHeader
{
  uint32_t _offsetOfTexture;
  uint32_t _compressedLen;   //Compressed length of texture
  uint32_t _fullLen;         //Full length of texture
  uint8_t  _type;
  uint8_t  _compressionType; // 0 - none of compression
  uint16_t _dummy;
  char     _name[16];
};
STRUCT_PACK_END
//-----------------------------------------------------------------------------
STRUCT_PACK_BEGIN
struct WadTextureHeader
{
  char     _name[16];
  uint32_t _width;
  uint32_t _height;
  uint32_t _offsetOfImage;
  uint32_t _offsetOfMipmap1;
  uint32_t _offsetOfMipmap2;
  uint32_t _offsetOfMipmap3;
};
STRUCT_PACK_END
//-----------------------------------------------------------------------------
STRUCT_PACK_BEGIN
struct WadTextureColor
{
  uint8_t _red;
  uint8_t _green;
  uint8_t _blue;
};
STRUCT_PACK_END
typedef std::vector<WadTextureColor> VecWadTextureColor;
//-----------------------------------------------------------------------------
struct WadTextureBody
{
  VecByte _image;
  VecByte _mipmap1;
  VecByte _mipmap2;
  VecByte _mipmap3;
  VecWadTextureColor _colors;
};
//-----------------------------------------------------------------------------
struct WadTexture
{
  WadTextureHeader _header;
  WadTextureBody _body;
  WadLumpHeader _lump;
};
typedef std::vector<WadTexture> VecWadTexture;
//-----------------------------------------------------------------------------
struct WadFileData
{
  WadHeader _header;
  VecWadTexture _textures;
};
//-----------------------------------------------------------------------------
class WadDecoder
{
public:
  bool decode(const VecByte & buffer, WadFileData & wadData) const;

private:
  bool decodeTextureBody(MemReader reader, WadTextureHeader & header, WadTextureBody & body) const;
};
//-----------------------------------------------------------------------------
#endif // WadDecoderH
