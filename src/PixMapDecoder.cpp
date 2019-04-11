#include "PixMapDecoder.h"
#include "MemReader.h"
#include <stdexcept>

PixMapDecoder::PixMapDecoder()
{}

bool PixMapDecoder::tryToDecode(const std::vector<uint8_t> & data, PixMapPicture & pic)
{
  std::string s((char*)data.data(), data.size());

  //P6\n
  size_t vOff = s.find("\n");
  if (vOff == std::string::npos)
    return false;

  pic._ver = getVersionFromStr(s.substr(0, vOff));
  if (pic._ver == PixMapVersion::Version_Unknown)
    return false;
  if (pic._ver != PixMapVersion::Version_P6)
  {
    TRACE(ERR) << "Version of pixmap is not supported: " << s.substr(0, vOff);
    return false;
  }

  //128\t
  size_t wOff = s.find(" ", vOff);
  if (wOff == std::string::npos)
    return false;
  try
  {
    pic._width = std::stoi(s.substr(vOff + 1, wOff - vOff + 1));
  }
  catch (std::invalid_argument)
  {
    return false;
  }
  catch (std::out_of_range)
  {
    return false;
  }

  //128\n
  size_t hOff = s.find("\n", wOff);
  if (hOff == std::string::npos)
    return false;
  try
  {
    pic._height = std::stoi(s.substr(wOff + 1, hOff - wOff + 1));
  }
  catch (std::invalid_argument)
  {
    return false;
  }
  catch (std::out_of_range)
  {
    return false;
  }

  //255\n
  size_t pOff = s.find("\n", hOff + 1);
  if (pOff == std::string::npos)
    return false;

  MemReader r((uint8_t*)s.data() + pOff + 1, s.size() - pOff + 1);
  if (r.getRemainingSize() < pic._height * pic._width * 3)
    return false;
  pic._red.resize(pic._height * pic._width);
  pic._green.resize(pic._height * pic._width);
  pic._blue.resize(pic._height * pic._width);
  for (size_t i = 0; i < (pic._height * pic._width); ++i)
  {
    r.readUint8(pic._red[i]);
    r.readUint8(pic._green[i]);
    r.readUint8(pic._blue[i]);
  }
  return true;
}

PixMapVersion PixMapDecoder::getVersionFromStr(const std::string & str) const
{
  if (str == "P1")
    return PixMapVersion::Version_P1;
  else if (str == "P2")
    return PixMapVersion::Version_P2;
  else if (str == "P3")
    return PixMapVersion::Version_P3;
  else if (str == "P4")
    return PixMapVersion::Version_P4;
  else if (str == "P5")
    return PixMapVersion::Version_P5;
  else if (str == "P6")
    return PixMapVersion::Version_P6;
  else
    return PixMapVersion::Version_Unknown;
}
