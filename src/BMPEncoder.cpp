#include "BMPEncoder.h"
#include "MemWriter.h"
#include "MemReader.h"
#include "Tracer.h"
#include "InetUtils.h"
#include <cstring>

BMPEncoder::BMPEncoder()
{}

bool BMPEncoder::encode(const BmpData & bmpData, VecByte & encoded)
{
  encoded.clear();
  uint32_t headersSize = sizeof(BmpFileHeader) + sizeof(BmpInfoHeader);
  uint32_t width = ((bmpData._width + 3) & ~3);
  uint32_t height = bmpData._height;
  uint32_t imgSize = width * height;
  uint32_t fileSize = headersSize + (bmpData._palette.size() * sizeof(Color)) + imgSize;

  encoded.clear();
  encoded.resize(fileSize);

  BmpFileHeader fileHeader;
  fileHeader._fileType = ('M' << 8) | ('B');
  fileHeader._fileSize = fileSize;
  fileHeader._dummy1 = 0;
  fileHeader._dummy2 = 0;
  fileHeader._dataOffset = headersSize + bmpData._palette.size() * sizeof(Color);

  MemWriter wr(encoded.data(), encoded.size());
  if (wr.writeData(&fileHeader, sizeof(BmpFileHeader)) == false)
  {
    TRACE(ERR) << "Can't write BMP file header";
    return false;
  }

  BmpInfoHeader infoHeader;
  memset(&infoHeader, 0, sizeof(infoHeader));
  infoHeader._headerSize = sizeof(BmpInfoHeader);
  infoHeader._width = width;
  infoHeader._height = height;
  infoHeader._clrPlanes = 1;
  infoHeader._bitsPerPixel = 8;
  infoHeader._compressionType = BmpCompressionType::Bmp_BI_RGB;
  infoHeader._imageSize = imgSize;
  infoHeader._horizontalResol = 0;
  infoHeader._verticalResol = 0;
  infoHeader._clrUsed = 256;
  infoHeader._importantClrUsed = 0;

  if (wr.writeData(&infoHeader, sizeof(BmpInfoHeader)) == false)
  {
    TRACE(ERR) << "Can't write BMP info header";
    return false;
  }

  MemReader rd((uint8_t *)bmpData._palette.data(), bmpData._palette.size() * sizeof(Color));

  for (int i = 0; i < (int)infoHeader._clrUsed; ++i)
  {
    Color clr;
    if (rd.readData((uint8_t *)&clr, sizeof(Color)) == false)
    {
      TRACE(ERR) << "Can't read color from palette, index: " << i;
      return false;
    }

    if (wr.writeData(&clr, sizeof(Color)) == false)
    {
      TRACE(ERR) << "Can't write color to BMP file, index: " << i;
      return false;
    }
  }

  wr = MemWriter(wr.getPos(), wr.getRemainingSize());

  const uint8_t * bmpImgData = bmpData._data.data();
  bmpImgData += (height - 1) * bmpData._width;
  for (uint32_t i = 0; i < height; ++i)
  {
    wr.seek(width * i);
    wr.writeData(bmpImgData, bmpData._width);
    bmpImgData -= bmpData._width;
  }
  return true;
}
