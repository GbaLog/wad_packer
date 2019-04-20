#ifndef WADDECODER_H
#define WADDECODER_H

#include <cstdint>
#include <vector>
#include "Tracer.h"
#include "MemReader.h"

bool isWadFormat(uint32_t magic);

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
  TextureItem(MemReader reader);

  std::string getName() const { return _params._name; }
  uint32_t getWidth() const { return _params._width; }
  uint32_t getHeight() const { return _params._height; }
  const std::vector<uint8_t> & getImage() const { return _image; }
  const std::vector<uint8_t> & getMipmap1() const { return _mipmap1; }
  const std::vector<uint8_t> & getMipmap2() const { return _mipmap2; }
  const std::vector<uint8_t> & getMipmap3() const { return _mipmap3; }
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

class WadDecoder
{
public:
  WadDecoder();
};

#endif // WADDECODER_H
