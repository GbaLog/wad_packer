#ifndef PixMapDecoderH
#define PixMapDecoderH

#include <vector>
#include <cstdint>
#include "Tracer.h"

//Only P6 supported
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

struct PixMapPicture
{
  PixMapVersion _ver;
  uint32_t  _height;
  uint32_t  _width;
  std::vector<uint8_t> _red;
  std::vector<uint8_t> _green;
  std::vector<uint8_t> _blue;
};

class PixMapDecoder
{
public:
  PixMapDecoder();

  bool tryToDecode(const std::vector<uint8_t> & data, PixMapPicture & pic);

private:
  PixMapVersion getVersionFromStr(const std::string & str) const;
};

#endif // PixMapDecoderH
