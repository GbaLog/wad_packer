cmake_minimum_required(VERSION 2.8)

project(wad_packer)

set(SRC_LIST
  main.cpp

  MemReader.cpp
  MemReader.h
  PixMapDecoder.cpp
  PixMapDecoder.h
)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} Ws2_32)
