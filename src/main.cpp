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
  TextureItem(const LumpItem & lump, const uint8_t * data, uint32_t size)
  {
    memcpy(&_params, data, sizeof(TextureItemParams));
    _image.resize(_params._width * _params._height);

    const uint8_t * dataImage = data + _params._offsetOfImage;
    std::copy(dataImage, dataImage + _image.size(), _image.begin());

    const uint8_t * dataRgb = data + _params._offsetOfMipmap3 + (_params._width / 8 * _params._height / 8) + 2;
    _rgb[0].resize(256);
    _rgb[1].resize(256);
    _rgb[2].resize(256);
    for (int i = 0; i < 256; ++i)
    {
      uint8_t colorRed = *dataRgb++;
      uint8_t colorGreen = *dataRgb++;
      uint8_t colorBlue = *dataRgb++;
      _rgb[0][i] = colorRed;
      _rgb[1][i] = colorGreen;
      _rgb[2][i] = colorBlue;
      //it = (0xFF << 24) | (colorRed << 16) | (colorBlue << 8) | colorGreen;
    }
  }

  TextureItem(MemReader & reader)
  {
    if (reader.readData((uint8_t*)&_params, sizeof(_params)) == false)
    {
      TRACE(ERR) << "Can't read texture params";
      throw std::runtime_error("Can't read texture params");
    }

    _image.resize(_params._width * _params._height);

    if (reader.shift(_params._offsetOfImage) == false)
    {
      TRACE(ERR) << "Can't shift to image";
      throw std::runtime_error("Can't shift to image");
    }

    reader.readData(_image.data(), _image.size());

    reader.shift(-(_image.size() + _params._offsetOfImage + sizeof(_params)));
    if (reader.shift(_params._offsetOfMipmap3 + (_params._width / 8 * _params._height / 8) + 2) == false)
    {
      TRACE(ERR) << "Can't shift to RGB map";
      throw std::runtime_error("Can't shift to RGB map");
    }

    _rgb[0].resize(256);
    _rgb[1].resize(256);
    _rgb[2].resize(256);
    for (int i = 0; i < 256; ++i)
    {
      uint8_t & colorRed   = _rgb[0][i];
      uint8_t & colorGreen = _rgb[1][i];
      uint8_t & colorBlue  = _rgb[2][i];
      reader.readUint8(colorRed);
      reader.readUint8(colorGreen);
      reader.readUint8(colorBlue);
    }
  }

//private:
  TextureItemParams _params;
  std::vector<uint8_t> _image;
  std::vector<uint8_t> _mipmap1;
  std::vector<uint8_t> _mipmap2;
  std::vector<uint8_t> _mipmap3;
  std::vector<uint8_t> _rgb[3];
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
    //memcpy(&lump, dataPtr, sizeof(LumpItem));
    TRACE(DBG) << "Lump: " << std::string(lump._name, 16);
    lumps.push_back(lump);

    dataPtr += sizeof(LumpItem);
  }

  std::vector<TextureItem> items;
  for (const auto & lump : lumps)
  {
    dataPtr = fullFile.data() + lump._offsetOfTexture;
    reader.seek(lump._offsetOfTexture);
    //TextureItem item(lump, dataPtr, fullFile.size() - lump._offsetOfTexture);
    TextureItem item(reader);
    items.push_back(item);
  }

  std::string dir = "decoded_"; dir += argv[1];
  system(std::string("rmdir /S /Q " + dir).c_str());
  system(std::string("mkdir " + dir).c_str());
  for (const auto & item : items)
  {
    std::string filename = dir + "/" + item._params._name;
    while (isspace(filename.back()) || filename.back() == 0x00) filename.pop_back();
    filename += ".ppm";
    TRACE(DBG) << "Filename: " << filename;
    std::ofstream decFile(filename, std::ios::out | std::ios::binary);
    if (!decFile)
    {
      TRACE(WRN) << "Cannot create file: " << dir + std::string(item._params._name, 16);
      continue;
    }

    decFile << "P6\n";
    decFile << item._params._width << " " << item._params._height << "\n255\n";

    for (const auto it : item._image)
    {
      decFile << (char)item._rgb[0][it] << (char)item._rgb[1][it] << (char)item._rgb[2][it];
    }

    /*
    for (uint32_t x = 0; x < item._params._width; ++x)
    {
      for (uint32_t y = 0; y < item._params._height; ++y)
      {
        decFile << (char)(item._image[x + (y * item._params._width)]);
      }
    }
    */
  }
}
