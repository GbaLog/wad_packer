#include "InetUtils.h"

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

namespace ratel
{

uint16_t htons(uint16_t val)
{
  return ::htons(val);
}

uint32_t htonl(uint32_t val)
{
  return ::htonl(val);
}

}
