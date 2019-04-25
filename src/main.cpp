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
#include "WadDecoder.h"
#include "BMPEncoder.h"
#include "BMPDecoder.h"
#include "MemWriter.h"

void encodePixMap(const std::string & filename, const std::vector<TextureItem> & items)
{
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

void encodeTextureToBmp(const std::string & filename, const TextureItem & item)
{
  const std::string imgName = filename + ".bmp";
  const std::string mipmap1Name = filename + "_mip1.bmp";
  const std::string mipmap2Name = filename + "_mip2.bmp";
  const std::string mipmap3Name = filename + "_mip3.bmp";

  BMPEncoder enc;

  BmpData bmpData;
  bmpData._width = item.getWidth();
  bmpData._height = item.getHeight();
  bmpData._data = item.getImage();

  if (item.getRed().size() != 256)
  {
    TRACE(WRN) << "Color palette is not 256: " << item.getRed().size();
    return;
  }

  bmpData._palette.reserve(256);
  for (int i = 0; i < 256; ++i)
  {
    Color clr;
    clr._red = item.getRed().at(i);
    clr._green = item.getGreen().at(i);
    clr._blue = item.getBlue().at(i);
    clr._reserved = 0;
    bmpData._palette.push_back(clr);
  }

  VecByte data;
  if (enc.encode(bmpData, data) == false)
  {
    TRACE(ERR) << "Can't encode BMP: " << imgName;
    return;
  }

  TRACE(DBG) << "BMP file size: " << data.size();

  std::ofstream decFile(imgName, std::ios::out | std::ios::binary);
  if (!decFile)
  {
    TRACE(WRN) << "Cannot create file: " << imgName;
    return;
  }

  decFile.write((char *)data.data(), data.size());
  decFile.close();

  decFile.open(mipmap1Name);
  if (!decFile)
  {
    TRACE(ERR) << "Cannot create mipmap1 file: " << mipmap1Name;
    return;
  }

  bmpData._width = item.getWidth() / 2;
  bmpData._height = item.getHeight() / 2;
  bmpData._data = item.getMipmap1();

  if (enc.encode(bmpData, data) == false)
  {
    TRACE(ERR) << "Can't encode BMP: " << mipmap1Name;
    return;
  }

  decFile.write((char *)data.data(), data.size());
  decFile.close();

  decFile.open(mipmap2Name);
  if (!decFile)
  {
    TRACE(ERR) << "Cannot create mipmap1 file: " << mipmap2Name;
    return;
  }

  bmpData._width = item.getWidth() / 4;
  bmpData._height = item.getHeight() / 4;
  bmpData._data = item.getMipmap2();

  if (enc.encode(bmpData, data) == false)
  {
    TRACE(ERR) << "Can't encode BMP: " << mipmap2Name;
    return;
  }

  decFile.write((char *)data.data(), data.size());
  decFile.close();

  decFile.open(mipmap3Name);
  if (!decFile)
  {
    TRACE(ERR) << "Cannot create mipmap1 file: " << mipmap3Name;
    return;
  }

  bmpData._width = item.getWidth() / 8;
  bmpData._height = item.getHeight() / 8;
  bmpData._data = item.getMipmap3();

  if (enc.encode(bmpData, data) == false)
  {
    TRACE(ERR) << "Can't encode BMP: " << mipmap3Name;
    return;
  }

  decFile.write((char *)data.data(), data.size());
  decFile.close();
}

void encodeTexturesToBmp(const std::string & filename, const std::vector<TextureItem> & items)
{
  std::string dir = "decoded_"; dir += filename;
  system(std::string("rmdir /S /Q " + dir).c_str());
  system(std::string("mkdir " + dir).c_str());

  for (const auto & item : items)
  {
    std::string filename = dir + "/" + item.getName();
    while (isspace(filename.back()) || filename.back() == 0x00) filename.pop_back();
    TRACE(DBG) << "Filename: " << filename;

    encodeTextureToBmp(filename, item);
  }
}

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

  if (isWadFormat(wh._magicWord) == false)
  {
    TRACE(ERR) << "It's not WAD format: " << std::string((char*)(&wh._magicWord), 4);
    return EXIT_FAILURE;
  }

  TRACE(DBG) << "WAD header: number of all textures: " << wh._numOfTextures << ", offset: " << wh._offsetOfLumps;

  reader.seek(wh._offsetOfLumps);

  //Load lumps
  std::vector<LumpItem> lumps;
  lumps.reserve(wh._numOfTextures);
  for (uint32_t i = 0; i < wh._numOfTextures; ++i)
  {
    LumpItem lump;
    reader.readData((uint8_t*)&lump, sizeof(LumpItem));
    lumps.push_back(lump);
  }

  std::vector<TextureItem> items;
  for (const auto & lump : lumps)
  {
    reader.seek(lump._offsetOfTexture);
    TextureItem item(MemReader(reader.getPos(), lump._fullLen));
    items.push_back(item);
  }

  //encodePixMap(filename, items);
  encodeTexturesToBmp(filename, items);
  return EXIT_SUCCESS;
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
  BMPDecoder dec;
  BmpData bmpData;
  if (dec.decode(fullFile, bmpData) == false)
  {
    TRACE(ERR) << "Can't decode picture in file: " << filename;
    return EXIT_FAILURE;
  }

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
  li._fullLen = 0;
  li._type = 0x43;
  li._compressionType = 0;
  li._dummy = 0;
  strncpy(li._name, name.c_str(), std::max(16ULL, name.size()));

  uint32_t w = bmpData._width;
  uint32_t h = bmpData._height;

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

  std::ofstream ofs(argv[1], std::ios::out | std::ios::binary);

  wh._offsetOfLumps = sizeof(WADHeader) + li._fullLen;

  ofs.write((char *)&wh, sizeof(wh));
  ofs.write((char *)&params, sizeof(params));

  ofs.write((char *)bmpData._data.data(), bmpData._data.size());

  for (size_t i = 0; i < ((h / 2) * (w / 2)); ++i)
  {
    ofs << (char)0x00;
  }

  for (size_t i = 0; i < ((h / 4) * (w / 4)); ++i)
  {
    ofs << (char)0x00;
  }

  for (size_t i = 0; i < ((h / 8) * (w / 8)); ++i)
  {
    ofs << (char)0x00;
  }

  ofs << (char)0x00 << (char)0x01;

  for (size_t i = 0; i < bmpData._palette.size(); ++i)
  {
    ofs.write((char *)&bmpData._palette[i], 3);
  }

  ofs << (char)0x00 << (char)0x00;

  ofs.write((char*)&li, sizeof(li));
  ofs.close();
}
