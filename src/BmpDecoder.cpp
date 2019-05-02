#include "BMPDecoder.h"
#include "MemReader.h"
#include "MemWriter.h"
#include "Tracer.h"
//-----------------------------------------------------------------------------
bool BmpDecoder::decode(const VecByte & bmpBytes, BmpData & data)
{
  MemReader rd(bmpBytes.data(), bmpBytes.size());

  BmpFileHeader fileHeader;
  if (rd.readData((uint8_t *)&fileHeader, sizeof(BmpFileHeader)) == false)
  {
    TRACE(ERR) << "Can't read BMP file header";
    return false;
  }

  BmpInfoHeader infoHeader;
  if (rd.readData((uint8_t *)&infoHeader, sizeof(BmpInfoHeader)) == false)
  {
    TRACE(ERR) << "Can't read BMP info header";
    return false;
  }

  if (!(infoHeader._headerSize == sizeof(BmpInfoHeader) && infoHeader._clrPlanes == 1))
  {
    TRACE(ERR) << "Header DIB header";
    return false;
  }

  if (!(fileHeader._dummy1 == 0 && fileHeader._dummy2 == 0))
  {
    TRACE(ERR) << "Wrong BMP file";
    return false;
  }

  if (infoHeader._bitsPerPixel != 8)
  {
    TRACE(ERR) << "BMP file is not 8 bit";
    return false;
  }

  uint32_t paletteBytes = 0;
  if (infoHeader._clrUsed == 0)
  {
    infoHeader._clrUsed = 256;
    paletteBytes = (1 << infoHeader._bitsPerPixel) * sizeof(Color);
  }
  else
  {
    paletteBytes = infoHeader._clrUsed * sizeof(Color);
  }

  data._width = (infoHeader._width + 3) & ~3;
  data._height = infoHeader._height;
  uint32_t imgSize = data._width * data._height;
  data._palette.resize(256);

  if (rd.readData((uint8_t *)data._palette.data(), data._palette.size() * sizeof(Color)) == false)
  {
    TRACE(ERR) << "Can't read BMP file palette";
    return false;
  }

  data._data.resize(imgSize);

  if (rd.getRemainingSize() != data._data.size())
  {
    TRACE(ERR) << "BMP data is not full: " << rd.getRemainingSize() << ", need: " << data._data.size();
    return false;
  }

  rd = MemReader(rd.getPos(), rd.getRemainingSize());

  uint8_t * bmpImgData = data._data.data();
  bmpImgData += (data._height - 1) * data._width;
  for (uint32_t i = 0; i < data._height; ++i)
  {
    rd.shiftFromStart(data._width * i);
    if (rd.readData(bmpImgData, data._width) == false)
    {
      TRACE(ERR) << "Can't read part of bmp image";
      return false;
    }
    bmpImgData -= data._width;
  }
  return true;
}
//-----------------------------------------------------------------------------
