cmake_minimum_required(VERSION 2.8)

project(wad_packer)

set(SRC_LIST
  main.cpp

  MemReader.cpp
  MemReader.h
  MemWriter.cpp
  MemWriter.h
  PixMapDecoder.cpp
  PixMapDecoder.h
  BMPEncoder.cpp
  BMPEncoder.h
  BMPDecoder.cpp
  BMPDecoder.h
  BMPCommon.h
  WadDecoder.cpp
  WadDecoder.h
  InetUtils.cpp
  InetUtils.h
  Tracer.h
)

set(CMAKE_CXX_STANDARD 14)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} Ws2_32)
