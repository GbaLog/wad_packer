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

bool readFile(const std::string & filename, std::vector<uint8_t> & data)
{
  std::ifstream file;
  file.open(filename, std::ios::in | std::ios::binary);
  if (!file)
  {
    TRACE(ERR) << "Can't open file for reading: " << filename;
    return false;
  }

  data.insert(data.end(), std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
  return true;
}

void crackPath(const std::string & filename, std::string & path, std::string & out)
{
  auto it = filename.find_last_of("\\/");
  if (it != std::string::npos)
  {
    path = filename.substr(0, it);
    out = filename.substr(it + 1);
  }
  else
  {
    path.clear();
    out = filename;
  }

  it = out.find_last_of('.');
  if (it != std::string::npos)
    out = out.substr(0, it);
}

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

  std::ofstream decFile(imgName, std::ios::out | std::ios::binary);
  if (!decFile)
  {
    TRACE(WRN) << "Cannot create file: " << imgName;
    return;
  }

  decFile.write((char *)data.data(), data.size());
  decFile.close();

  decFile.open(mipmap1Name, std::ios::out | std::ios::binary);
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

  decFile.open(mipmap2Name, std::ios::out | std::ios::binary);
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

  decFile.open(mipmap3Name, std::ios::out | std::ios::binary);
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
  TRACE(DBG) << "Decode wad: " << filename;
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

bool processItem(const std::string & filepath, std::vector<uint8_t> & ofs, LumpItem & li)
{
  std::string filename = filepath;
  std::ifstream ifs(filename, std::ios::in | std::ios::binary);

  if (!ifs)
  {
    TRACE(ERR) << "Can't open texture file: " << filename;
    return EXIT_FAILURE;
  }

  std::vector<uint8_t> fullFile{std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
  BMPDecoder dec;
  BmpData bmpData;
  if (dec.decode(fullFile, bmpData) == false)
  {
    TRACE(ERR) << "Can't decode picture in file: " << filename;
    return false;
  }

  std::string name;
  std::string path;
  crackPath(filename, path, name);
  if (name.size() > 16)
  {
    TRACE(ERR) << "File name size is greater than 16";
    return false;
  }

  memset(&li, 0, sizeof(li));
  li._offsetOfTexture = sizeof(WADHeader);
  li._compressedLen = 0;
  li._fullLen = 0;
  li._type = 0x43;
  li._compressionType = 0;
  li._dummy = 0;
  strncpy(li._name, name.c_str(), name.size());

  uint32_t w = bmpData._width;
  uint32_t h = bmpData._height;

  TextureItemParams params;
  memset(&params, 0, sizeof(params));
  strncpy(params._name, name.c_str(), name.size());
  params._width = w;
  params._height = h;
  params._offsetOfImage = sizeof(TextureItemParams);
  params._offsetOfMipmap1 = params._offsetOfImage + (w * h);
  params._offsetOfMipmap2 = params._offsetOfMipmap1 + ((w / 2) * (h / 2));
  params._offsetOfMipmap3 = params._offsetOfMipmap2 + ((w / 4) * (h / 4));

  li._fullLen = params._offsetOfMipmap3 + ((w / 8) * (h / 8)) + 2 + (256 * 3) + 2;
  li._compressedLen = li._fullLen;

  ofs.resize(li._fullLen);
  MemWriter wr(ofs.data(), ofs.size());

  wr.writeData(&params, sizeof(params));

  wr.writeData(bmpData._data.data(), bmpData._data.size());

  std::vector<uint8_t> mipmap1;
  std::vector<uint8_t> mipmap2;
  std::vector<uint8_t> mipmap3;
  BmpData mipmapData;

  if (!readFile(path + "/" + name + "_mip1.bmp", mipmap1) ||
      dec.decode(mipmap1, mipmapData) == false ||
      mipmapData._data.size() != ((h / 2) * (w / 2)))
  {
    TRACE(ERR) << "Can't read 1st mipmap";
    return false;
  }

  wr.writeData(mipmapData._data.data(), mipmapData._data.size());

  if (!readFile(path + "/" + name + "_mip2.bmp", mipmap2) ||
      dec.decode(mipmap2, mipmapData) == false ||
      mipmapData._data.size() != ((h / 4) * (w / 4)))
  {
    TRACE(ERR) << "Can't read 2nd mipmap";
    return false;
  }

  wr.writeData(mipmapData._data.data(), mipmapData._data.size());

  if (!readFile(path + "/" + name + "_mip3.bmp", mipmap3) ||
      dec.decode(mipmap3, mipmapData) == false)
  {
    TRACE(ERR) << "Can't read 3rd mipmap, data size: " << mipmapData._data.size()
               << ", expected size: " << ((h / 8) * (w / 8));
    return false;
  }

  if (mipmapData._data.size() != ((h / 8) * (w / 8)))
  {
    auto copyData = mipmapData._data;
    const uint8_t * ptr = mipmapData._data.data();
    for (uint32_t i = 9; i < h; ++i)
    {
      copyData.insert(copyData.end(), ptr, ptr + w);
      ptr += (w + 3) &  ~3;
    }
    mipmapData._data = copyData;
  }

  wr.writeData(mipmapData._data.data(), mipmapData._data.size());

  wr.writeUint8(0x00);
  wr.writeUint8(0x01);

  TRACE(DBG) << "Pallete size: " << bmpData._palette.size();
  for (size_t i = 0; i < bmpData._palette.size(); ++i)
  {
    wr.writeUint8(bmpData._palette[i]._red);
    wr.writeUint8(bmpData._palette[i]._green);
    wr.writeUint8(bmpData._palette[i]._blue);
  }

  wr.writeUint8(0x00);
  wr.writeUint8(0x00);
  return true;
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

  WADHeader wh;
  wh._magicWord = (('3' << 24) | ('D' << 16) | ('A' << 8) | 'W');
  wh._numOfTextures = argc - 2;

  std::vector<LumpItem> lumps;
  std::vector<std::vector<uint8_t> > datas;

  lumps.reserve(wh._numOfTextures);
  datas.reserve(wh._numOfTextures);

  for (int arg = 2; arg < argc; ++arg)
  {
    LumpItem lump;
    std::vector<uint8_t> data;
    if (processItem(argv[arg], data, lump) == false)
    {
      TRACE(ERR) << "Can't process texture: " << argv[arg];
      return EXIT_FAILURE;
    }
    lumps.push_back(lump);
    datas.push_back(data);
  }

  uint32_t fullLumpLen = 0;
  for (const auto & it : lumps)
    fullLumpLen += it._fullLen;

  wh._offsetOfLumps = sizeof(WADHeader) + fullLumpLen;

  std::ofstream ofs(argv[1], std::ios::out | std::ios::binary);
  if (!ofs)
  {
    TRACE(ERR) << "Can't open file for writing: " << argv[1];
    return EXIT_FAILURE;
  }

  ofs.write((char *)&wh, sizeof(wh));

  for (const auto & data : datas)
  {
    ofs.write((char *)data.data(), data.size());
  }

  for (const auto & lump : lumps)
  {
    ofs.write((char *)&lump, sizeof(lump));
  }
}
