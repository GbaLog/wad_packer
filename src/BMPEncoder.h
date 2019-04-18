#ifndef BMPEncoderH
#define BMPEncoderH

#include <cstdint>
#include <vector>

typedef std::vector<uint8_t> VecByte;

struct BmpFileHeader
{
  uint16_t _fileType;
  uint32_t _fileSize;
  uint16_t _dummy1;
  uint16_t _dummy2;
  uint32_t _dataOffset;
} __attribute__((packed));

struct BmpCompressionType
{
  enum
  {
    Bmp_BI_RGB = 0,
    Bmp_BI_RLE8 = 1,
    Bmp_BI_RLE4 = 2,
    Bmp_BI_BITFIELDS = 3,
    Bmp_BI_JPEG = 4,
    Bmp_BI_PNG = 5,
    Bmp_BI_ALPHABITFIELDS = 6,
    Bmp_BI_CMYK = 11,
    Bmp_BI_CMYKRLE8 = 12,
    Bmp_BI_CMYKRLE4 = 13
  };
};

struct BmpInfoHeader
{
  uint32_t _headerSize;
  uint32_t _width;
  uint32_t _height;
  uint16_t _clrPlanes;
  uint16_t _bitsPerPixel;
  uint32_t _compressionType;
  uint32_t _imageSize;
  uint32_t _horizontalResol;
  uint32_t _verticalResol;
  uint32_t _clrUsed;
  uint32_t _importantClrUsed;
} __attribute__((packed));

struct Color
{
  uint8_t _blue;
  uint8_t _green;
  uint8_t _red;
  uint8_t _reserved;
};

struct BmpData
{
  size_t _width;
  size_t _height;
  std::vector<Color> _palette;
  std::vector<uint8_t> _data;
};

class BMPEncoder
{
public:
  BMPEncoder();

  bool encode(const BmpData & bmpData, VecByte & encoded);
};

#endif // BMPEncoderH
