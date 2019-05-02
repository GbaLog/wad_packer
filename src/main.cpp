#include <fstream>
#include <cstring>
#include "Tracer.h"
#include "MemReader.h"
#include "PixMapDecoder.h"
#include "WadDecoder.h"
#include "BmpEncoder.h"
#include "BmpDecoder.h"
#include "MemWriter.h"
#include "Common.h"
//-----------------------------------------------------------------------------
bool readFile(const std::string & filename, VecByte & data)
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
//-----------------------------------------------------------------------------
void crackPath(const std::string & filename, std::string & path, std::string & file)
{
  auto it = filename.find_last_of("\\/");
  if (it != std::string::npos)
  {
    path = filename.substr(0, it);
    file = filename.substr(it + 1);
  }
  else
  {
    path.clear();
    file = filename;
  }

  it = file.find_last_of('.');
  if (it != std::string::npos)
    file = file.substr(0, it);
}
//-----------------------------------------------------------------------------
void encodeBmpFile(const std::string & filename, const BmpData & data)
{
  VecByte buffer;
  BMPEncoder enc;
  if (enc.encode(data, buffer) == false)
  {
    TRACE(ERR) << "Can't encode BMP data for file: " << filename;
    return;
  }

  std::ofstream decFile(filename, std::ios::out | std::ios::binary);
  if (!decFile)
  {
    TRACE(WRN) << "Cannot create file: " << filename;
    return;
  }

  decFile.write((char *)buffer.data(), buffer.size());
}
//-----------------------------------------------------------------------------
void encodeTextureToBmp(const std::string & filename, const WadTexture & item)
{
  const std::string imgName = filename + ".bmp";
  const std::string mipmap1Name = filename + "_mip1.bmp";
  const std::string mipmap2Name = filename + "_mip2.bmp";
  const std::string mipmap3Name = filename + "_mip3.bmp";

  BMPEncoder enc;

  BmpData bmpData;
  bmpData._width = item._header._width;
  bmpData._height = item._header._height;
  bmpData._data = item._body._image;

  if (item._body._colors.size() != 256)
  {
    TRACE(WRN) << "Color palette is not 256: " << item._body._colors.size();
    return;
  }

  bmpData._palette.reserve(256);
  for (int i = 0; i < 256; ++i)
  {
    const auto & itemColors = item._body._colors;
    Color clr;
    clr._red = itemColors.at(i)._red;
    clr._green = itemColors.at(i)._green;
    clr._blue = itemColors.at(i)._blue;
    clr._reserved = 0;
    bmpData._palette.push_back(clr);
  }

  encodeBmpFile(imgName, bmpData);

  bmpData._width = item._header._width / 2;
  bmpData._height = item._header._height / 2;
  bmpData._data = item._body._mipmap1;

  encodeBmpFile(mipmap1Name, bmpData);

  bmpData._width = item._header._width / 4;
  bmpData._height = item._header._height / 4;
  bmpData._data = item._body._mipmap2;

  encodeBmpFile(mipmap2Name, bmpData);

  bmpData._width = item._header._width / 8;
  bmpData._height = item._header._height / 8;
  bmpData._data = item._body._mipmap3;

  encodeBmpFile(mipmap3Name, bmpData);
}
//-----------------------------------------------------------------------------
void encodeTexturesToBmp(const std::string & filename, const std::vector<WadTexture> & items)
{
  std::string dir = "decoded_"; dir += filename;
  system(std::string("rmdir /S /Q " + dir).c_str());
  system(std::string("mkdir " + dir).c_str());

  for (const auto & item : items)
  {
    std::string filename = dir + "/" + item._header._name;
    TRACE(DBG) << "Filename: " << filename;

    encodeTextureToBmp(filename, item);
  }
}
//-----------------------------------------------------------------------------
bool decodeWad(const std::string & filename)
{
  TRACE(DBG) << "Decode wad: " << filename;
  std::ifstream file;
  file.open(filename, std::ios::in | std::ios::binary);
  if (!file)
  {
    TRACE(ERR) << "Can't open file: " << filename;
    return false;
  }

  VecByte fullFile{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
  WadFileData fileData;
  WadDecoder dec;
  if (dec.decode(fullFile, fileData) == false)
  {
    TRACE(ERR) << "Can't decode wad file: " << filename;
    return false;
  }

  TRACE(DBG) << "WAD header: number of all textures: " << fileData._header._numOfTextures;

  encodeTexturesToBmp(filename, fileData._textures);
  return true;
}
//-----------------------------------------------------------------------------
void normalizeBmpData(VecByte & data, uint32_t width, uint32_t height)
{
  std::vector<uint8_t> copyData;
  const uint8_t * ptr = data.data();
  for (uint32_t i = 0; i < height; ++i)
  {
    copyData.insert(copyData.end(), ptr, ptr + width);
    ptr += (width + 3) &  ~3;
  }

  data = std::move(copyData);
}
//-----------------------------------------------------------------------------
bool loadTexture(const std::string & filename, WadTexture & texture)
{
  std::string name;
  std::string path;
  crackPath(filename, path, name);
  if (name.size() > 16)
  {
    TRACE(ERR) << "File name size is greater than 16";
    return false;
  }

  WadLumpHeader & li = texture._lump;
  memset(&li, 0, sizeof(li));
  li._offsetOfTexture = sizeof(WadHeader);
  li._compressedLen = 0;
  li._fullLen = 0;
  li._type = 0x43;
  li._compressionType = 0;
  li._dummy = 0;
  strncpy(li._name, name.c_str(), name.size());

  if (readFile(filename, texture._body._image) == false)
  {
    TRACE(ERR) << "Can't read original image: " << filename;
    return false;
  }

  BmpDecoder dec;
  BmpData bmpData;
  if (dec.decode(texture._body._image, bmpData) == false)
  {
    TRACE(ERR) << "Can't decode picture in file: " << filename;
    return false;
  }

  uint32_t w = bmpData._width;
  uint32_t h = bmpData._height;
  texture._body._image = std::move(bmpData._data);

  WadTextureHeader & header = texture._header;
  memset(&header, 0, sizeof(header));
  strncpy(header._name, name.c_str(), name.size());
  header._width = w;
  header._height = h;
  header._offsetOfImage = sizeof(WadTextureHeader);
  header._offsetOfMipmap1 = header._offsetOfImage + (w * h);
  header._offsetOfMipmap2 = header._offsetOfMipmap1 + ((w / 2) * (h / 2));
  header._offsetOfMipmap3 = header._offsetOfMipmap2 + ((w / 4) * (h / 4));

  li._fullLen = header._offsetOfMipmap3 + ((w / 8) * (h / 8)) + 2 + (256 * 3) + 2;
  li._compressedLen = li._fullLen;

  const std::string mipmap1Name = path + "/" + name + "_mip1.bmp";
  const std::string mipmap2Name = path + "/" + name + "_mip2.bmp";
  const std::string mipmap3Name = path + "/" + name + "_mip3.bmp";

  if (readFile(mipmap1Name, texture._body._mipmap1) == false ||
      readFile(mipmap2Name, texture._body._mipmap2) == false ||
      readFile(mipmap3Name, texture._body._mipmap3) == false)
  {
    TRACE(ERR) << "Can't read one of mipmaps";
    return false;
  }
  return true;
}
//-----------------------------------------------------------------------------
void encodeTextureToVecByte(const WadTexture & texture, VecByte & fullTextureData)
{
  fullTextureData.clear();
  fullTextureData.resize(texture._lump._fullLen);

  MemWriter wr(fullTextureData.data(), fullTextureData.size());
  const auto & body = texture._body;

  wr.writeData(&texture._header, sizeof(WadTextureHeader));
  wr.writeData(body._image.data(), body._image.size());

  uint32_t w = texture._header._width;
  uint32_t h = texture._header._height;

  BmpDecoder dec;
  BmpData mipmap1;
  BmpData mipmap2;
  BmpData mipmap3;

  if (dec.decode(body._mipmap1, mipmap1) == false ||
      dec.decode(body._mipmap2, mipmap2) == false ||
      dec.decode(body._mipmap3, mipmap3) == false)
  {
    TRACE(ERR) << "Can't decode one of mipmap";
    return;
  }

  if (mipmap1._data.size() != getMipmap1Size(w, h))
  {
    normalizeBmpData(mipmap1._data, w / 2, h / 2);
  }

  if (mipmap2._data.size() != getMipmap2Size(w, h))
  {
    normalizeBmpData(mipmap2._data, w / 4, h / 4);
  }

  if (mipmap3._data.size() != getMipmap3Size(w, h))
  {
    normalizeBmpData(mipmap3._data, w / 8, h / 8);
  }

  wr.writeData(mipmap1._data.data(), mipmap1._data.size());
  wr.writeData(mipmap2._data.data(), mipmap2._data.size());
  wr.writeData(mipmap3._data.data(), mipmap3._data.size());

  wr.writeUint8(0x00);
  wr.writeUint8(0x01);

  //Palette must be the same for all images
  for (size_t i = 0; i < mipmap1._palette.size(); ++i)
  {
    wr.writeUint8(mipmap1._palette[i]._red);
    wr.writeUint8(mipmap1._palette[i]._green);
    wr.writeUint8(mipmap1._palette[i]._blue);
  }

  wr.writeUint8(0x00);
  wr.writeUint8(0x00);
}
//-----------------------------------------------------------------------------
bool encodeToWad(const std::string & wadFilename, const std::string & path, VecString files)
{
  if (path.empty() == false)
  {
    for (auto & it : files)
      it = path + "/" + it;
  }

  WadHeader wh;
  wh._magicWord = (('3' << 24) | ('D' << 16) | ('A' << 8) | 'W');
  wh._numOfTextures = files.size();

  VecWadTexture textures;
  textures.reserve(wh._numOfTextures);

  uint32_t fullLumpLen = 0;
  for (const auto & file : files)
  {
    TRACE(DBG) << "Process file: " << file;

    WadTexture texture;
    if (loadTexture(file, texture) == false)
    {
      TRACE(ERR) << "Can't load texture: " << file;
      return false;
    }

    texture._lump._offsetOfTexture += fullLumpLen;
    fullLumpLen += texture._lump._fullLen;
    textures.push_back(texture);
  }

  wh._offsetOfLumps = sizeof(WadHeader) + fullLumpLen;

  std::ofstream ofs(wadFilename, std::ios::out | std::ios::binary);
  if (!ofs)
  {
    TRACE(ERR) << "Can't open file for writing: " << wadFilename;
    return false;
  }

  ofs.write((char *)&wh, sizeof(wh));

  for (const auto & texture : textures)
  {
    VecByte textureData;
    encodeTextureToVecByte(texture, textureData);
    ofs.write((char *)textureData.data(), textureData.size());
  }

  for (const auto & texture : textures)
  {
    ofs.write((char *)&texture._lump, sizeof(texture._lump));
  }
  return true;
}
//-----------------------------------------------------------------------------
int main(int argc, char * argv[])
{
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " <filename> for unpacking\n"
              << "       " << argv[0] << " <wad filename> [--path path] file1.bmp path/file2.bmp path/file3.bmp"
              << " for packing(path required if `--path` option is not used)\n";
    return EXIT_FAILURE;
  }

  if (argc == 2)
  {
    if (decodeWad(argv[1]) == false)
      return EXIT_FAILURE;
    return EXIT_SUCCESS;
  }

  std::string path;
  VecString files;
  for (int arg = 2; arg < argc; ++arg)
  {
    if (strcmp(argv[arg], "--path") == 0)
    {
      path = argv[++arg];
      continue;
    }

    files.push_back(argv[arg]);
  }

  if (encodeToWad(argv[1], path, std::move(files)) == false)
    return EXIT_FAILURE;
  return EXIT_SUCCESS;
}
