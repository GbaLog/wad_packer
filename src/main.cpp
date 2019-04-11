#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <set>
#include <algorithm>
#include <winsock2.h>
#include "Tracer.h"
#include "MemReader.h"
#include "PixMapDecoder.h"

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
} __attribute__((packed));

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

    uint32_t * w = &_params._width;
    uint32_t * h = &_params._height;

    reader.readData(_image.data(), _image.size());
    _mipmap1.resize((*w / 2) * (*h / 2));
    reader.readData(_mipmap1.data(), _mipmap1.size());
    _mipmap2.resize((*w / 4) * (*h / 4));
    reader.readData(_mipmap2.data(), _mipmap2.size());
    _mipmap3.resize((*w / 8) * (*h / 8));
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

int decodeWad(const std::string & filename)
{
  std::fstream file;
  file.open(filename, std::ios::in | std::ios::binary);
  if (!file)
  {
    TRACE(ERR) << "Can't open file: " << filename;
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

  std::string dir = "decoded_"; dir += filename;
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

int main(int argc, char * argv[])
{
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " <filename>\n";
    return EXIT_FAILURE;
  }

  if (argc == 2)
    return decodeWad(argv[1]);

  std::string filename = argv[2];
  std::ifstream ifs(filename, std::ios::in | std::ios::binary);

  if (!ifs)
  {
    TRACE(ERR) << "Can't open texture file: " << argv[2];
    return EXIT_FAILURE;
  }

  std::vector<uint8_t> fullFile{std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
  PixMapPicture pic;
  PixMapDecoder dec;
  if (dec.tryToDecode(fullFile, pic) == false)
  {
    TRACE(ERR) << "Can't decode picture in file: " << filename;
    return EXIT_FAILURE;
  }

  std::set<uint32_t> uniqueColors;
  for (size_t i = 0; i < pic._red.size(); ++i)
  {
    uniqueColors.insert((0x00 << 24) | (pic._red[i] << 16) | (pic._green[i] << 8) | (pic._blue[i]));
  }
  TRACE(INF) << "Unique colors: " << uniqueColors.size();

  WADHeader wh;
  wh._magicWord = (('3' << 24) | ('D' << 16) | ('A' << 8) | 'W');
  wh._numOfTextures = 1;

  std::string name;
  size_t off = filename.find_last_of("/\\");
  if (off == std::string::npos)
    name = filename.substr(0, filename.find("."));
  else
    name = filename.substr(off + 1, filename.find(".") - off + 1);

  LumpItem li;
  li._offsetOfTexture = sizeof(WADHeader);
  li._compressedLen = 0;
  li._fullLen = 0;\
  li._type = 0x43;
  li._compressionType = 0;
  li._dummy = 0;
  strncpy(li._name, name.c_str(), std::max(16ULL, name.size()));

  uint32_t & w = pic._width;
  uint32_t & h = pic._height;

  TextureItemParams params;
  strncpy(params._name, name.c_str(), std::max(16ULL, name.size()));
  params._width = w;
  params._height = h;
  params._offsetOfImage = sizeof(TextureItemParams);
  params._offsetOfMipmap1 = params._offsetOfImage + (w * h);
  params._offsetOfMipmap2 = params._offsetOfMipmap1 + ((w / 2) * (h / 2));
  params._offsetOfMipmap3 = params._offsetOfMipmap2 + ((w / 4) * (h / 4));

  li._fullLen = params._offsetOfMipmap3 + ((w / 8) * (h / 8)) + 2 + (256 * 3) + 2;
  li._compressedLen = li._fullLen;

  std::vector<uint32_t> colors;
  colors.reserve(256);
  std::copy(uniqueColors.begin(), uniqueColors.end(), std::back_inserter(colors));
  for (size_t i = uniqueColors.size(); i < 256; ++i)
    colors.push_back(0);

  std::ofstream ofs(argv[1], std::ios::out | std::ios::binary);

  wh._offsetOfLumps = sizeof(WADHeader) + li._fullLen;

  ofs.write((char *)&wh, sizeof(wh));
  ofs.write((char *)&params, sizeof(params));

  for (size_t i = 0; i < pic._red.size(); ++i)
  {
    auto it = std::find(colors.begin(), colors.end(), ((0x00 << 24) | (pic._red[i] << 16) | (pic._green[i] << 8) | (pic._blue[i])));
    if (it != colors.end())
      ofs << (char)(colors.begin() - it);
  }

  for (size_t i = 0; i < ((pic._height / 2) * (pic._width / 2)); ++i)
  {
    ofs << (char)0x00;
  }

  for (size_t i = 0; i < ((pic._height / 4) * (pic._width / 4)); ++i)
  {
    ofs << (char)0x00;
  }

  for (size_t i = 0; i < ((pic._height / 8) * (pic._width / 8)); ++i)
  {
    ofs << (char)0x00;
  }

  ofs << (char)0x00 << (char)0x01;
  for (const auto it : colors)
  {
    ofs << (char)(it >> 16);
    ofs << (char)(it >> 8);
    ofs << (char)(it);
  }

  ofs << (char)0x00 << (char)0x00;

  ofs.write((char*)&li, sizeof(li));
  ofs.close();
}
