cmake_minimum_required(VERSION 2.8)

project(wad_converter)

set(SRC_LIST
  main.cpp

  MemReader.cpp
  MemReader.h
)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} Ws2_32)
