# WAD Packer
WAD3 packer and unpacker

WAD3 is file format for Half-Life 1, Counter-Strike 1.6, etc that built on GoldSrc engine.
This project made for unpacking textures from WAD files.
Also it can pack them back if it is needed.
This can be useful for creating your own logos.

Features:
  - Encode BMP files to WAD
  - Decode from WAD to BMP files
  - Support of most common WAD type 43.
  
Requiremets for encoding BMP files to WAD:
  - BMP should have 256 or lesser color palette
  - BMP should be 8-bit bits per pixel
  - BMP shouldn't have compression
  - You have to create also BMP files with mipmaps. These are reduced copies of original images.
  Mipmap1 should have half the width and height of the original image. It should be named as original BMP + "_mip1" postfix.
  Mipmap2 should have four times less than the original image. It should be named as original BMP + "_mip2" postfix.
  Mipmap3 should have eight times smaller than the original image. It should be named as original BMP + "_mip3" postfix.
  - Requirements for mipmaps the same as for original image.
  - Texture name length should be 16 or lesser letters
  
To do:
  - Support of 40 and 42 types of WAD files
  - Support WAD2

Build:
  - `cd src`
  - 'mkdir build && cd build`
  - 'cmake ../`
  - `make`
  
Use:  
  For unpacking:  
    `./wad_packer.exe [file.wad]`  
  For packing:  
    `./wad_packer.exe file.wad path/file1.bmp path/file2.bmp path/file3.bmp`  
    or  
    `./wad_packer.exe file.wad --path path file1.bmp file2.bmp file3.bmp`  
  
