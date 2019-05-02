cmake_minimum_required(VERSION 2.8)

project(wad_packer)

set(SRC_LIST
  main.cpp
  PixMapDecoder.cpp
  PixMapDecoder.h
  BmpEncoder.cpp
  BmpEncoder.h
  BmpDecoder.cpp
  BmpDecoder.h
  BmpCommon.h
  WadDecoder.cpp
  WadDecoder.h
)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(utils)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} ratel-lib)
target_include_directories(${PROJECT_NAME} PRIVATE utils)
