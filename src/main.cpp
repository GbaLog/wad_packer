#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <winsock2.h>
#include "Tracer.h"
#include "MemReader.h"

bool isWadFormat(uint32_t magic)
{
  const uint32_t wadFormat = (0x00 << 24) | ('D' << 16) | ('A' << 8) | 'W';
  return (magic & 0x00FFFFFF) == wadFormat;
}

struct WADHeader
{
  uint32_t _magicWord;
  uint32_t _numOfTextures;
  uint32_t _offsetOfLumps;
} __attribute__((packed));

struct LumpItem
{
  uint32_t _offsetOfTexture;
  uint32_t _compressedLen;   //Compressed length of texture
  uint32_t _fullLen;         //Full length of texture
  uint8_t  _type;
  uint8_t  _compressionType; // 0 - none of compression
  uint16_t _dummy;
  char     _name[16];
} __attribute__((packed));

struct TextureItemParams
{
  char     _name[16];
  uint32_t _width;
  uint32_t _height;
  uint32_t _offsetOfImage;
  uint32_t _offsetOfMipmap1;
  uint32_t _offsetOfMipmap2;
  uint32_t _offsetOfMipmap3;
};

class TextureItem
{
public:
  TextureItem(MemReader reader)
  {
    if (reader.readData((uint8_t*)&_params, sizeof(_params)) == false)
    {
      TRACE(ERR) << "Can't read texture params";
      throw std::runtime_error("Can't read texture params");
    }

    _image.resize(_params._width * _params._height);

    if (reader.shiftFromStart(_params._offsetOfImage) == false)
    {
      TRACE(ERR) << "Can't shift to image";
      throw std::runtime_error("Can't shift to image");
    }

    uint32_t & w = _params._width;
    uint32_t & h = _params._height;

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

    if (reader.getRemainingSize() < 256 * 3)
    {
      TRACE(ERR) << "Can't load color palette";
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

  std::string getName() const { return _params._name; }
  uint32_t getWidth() const { return _params._width; }
  uint32_t getHeight() const { return _params._height; }
  const std::vector<uint8_t> & getImage() const { return _image; }
  const std::vector<uint8_t> & getRed() const { return _redColor; }
  const std::vector<uint8_t> & getGreen() const { return _greenColor; }
  const std::vector<uint8_t> & getBlue() const { return _blueColor; }

private:
  TextureItemParams _params;
  std::vector<uint8_t> _image;
  std::vector<uint8_t> _mipmap1;
  std::vector<uint8_t> _mipmap2;
  std::vector<uint8_t> _mipmap3;
  std::vector<uint8_t> _redColor;
  std::vector<uint8_t> _greenColor;
  std::vector<uint8_t> _blueColor;
};

int main(int argc, char * argv[])
{
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " filename\n";
    return EXIT_FAILURE;
  }

  std::fstream file;
  file.open(argv[1], std::ios::in | std::ios::binary);
  if (!file)
  {
    TRACE(ERR) << "Can't open file: " << argv[1];
    return EXIT_FAILURE;
  }

  std::vector<uint8_t> fullFile{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
  TRACE(DBG) << "File size: " << fullFile.size();

  MemReader reader(fullFile.data(), fullFile.size());

  WADHeader wh;
  reader.readData((uint8_t*)&wh, sizeof(WADHeader));
  //memcpy(&wh, fullFile.data(), sizeof(WADHeader));

  if (isWadFormat(wh._magicWord) == false)
  //if (strncmp((char*)(&wh._magicWord), "WAD", 3) != 0)
  {
    TRACE(ERR) << "It's not WAD format: " << std::string((char*)(&wh._magicWord), 4);
    return EXIT_FAILURE;
  }

  TRACE(DBG) << "WAD header: number of all textures: " << wh._numOfTextures << ", offset: " << wh._offsetOfLumps;

  uint8_t * dataPtr = fullFile.data() + wh._offsetOfLumps;
  reader.seek(wh._offsetOfLumps);

  //Load lumps
  std::vector<LumpItem> lumps;
  lumps.reserve(wh._numOfTextures);
  for (uint32_t i = 0; i < wh._numOfTextures; ++i)
  {
    LumpItem lump;
    reader.readData((uint8_t*)&lump, sizeof(LumpItem));
    lumps.push_back(lump);

    dataPtr += sizeof(LumpItem);
  }

  std::vector<TextureItem> items;
  for (const auto & lump : lumps)
  {
    dataPtr = fullFile.data() + lump._offsetOfTexture;
    reader.seek(lump._offsetOfTexture);
    TextureItem item(MemReader(reader.getPos(), reader.getRemainingSize()));
    items.push_back(item);
  }

  std::string dir = "decoded_"; dir += argv[1];
  system(std::string("rmdir /S /Q " + dir).c_str());
  system(std::string("mkdir " + dir).c_str());
  for (const auto & item : items)
  {
    std::string filename = dir + "/" + item.getName();
    while (isspace(filename.back()) || filename.back() == 0x00) filename.pop_back();
    filename += ".ppm";
    TRACE(DBG) << "Filename: " << filename;
    std::ofstream decFile(filename, std::ios::out | std::ios::binary);
    if (!decFile)
    {
      TRACE(WRN) << "Cannot create file: " << filename;
      continue;
    }

    decFile << "P6\n";
    decFile << item.getWidth() << " " << item.getHeight() << "\n255\n";

    for (const auto it : item.getImage())
    {
      decFile << (char)item.getRed()[it] << (char)item.getGreen()[it] << (char)item.getBlue()[it];
    }
  }
}
