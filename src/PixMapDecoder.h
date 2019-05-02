#ifndef PixMapDecoderH
#define PixMapDecoderH
//-----------------------------------------------------------------------------
#include <cstdint>
#include <string>
#include "Common.h"
//-----------------------------------------------------------------------------
enum PixMapVersion
{
  Version_Unknown,
  Version_P1,
  Version_P2,
  Version_P3,
  Version_P4,
  Version_P5,
  Version_P6
};
//-----------------------------------------------------------------------------
struct PixMapPicture
{
  PixMapVersion _version;
  uint32_t  _height;
  uint32_t  _width;
  VecByte _red;
  VecByte _green;
  VecByte _blue;
};
//-----------------------------------------------------------------------------
class PixMapDecoder
{
public:
  bool decode(const VecByte & data, PixMapPicture & pic) const;

private:
  PixMapVersion getVersionFromStr(const std::string & str) const;
};
//-----------------------------------------------------------------------------
#endif // PixMapDecoderH
//-----------------------------------------------------------------------------
