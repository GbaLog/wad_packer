add_library(ratel-lib ${LIBTYPE}
  InetUtils.cpp
  InetUtils.h
  MemReader.cpp
  MemReader.h
  MemWriter.cpp
  MemWriter.h
  Tracer.h
  Common.h
)

if (WIN32)
  target_link_libraries(ratel-lib Ws2_32)
endif()
