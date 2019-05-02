#ifndef RatelInetUtilsH
#define RatelInetUtilsH
//-----------------------------------------------------------------------------
#include <cstdint>
//-----------------------------------------------------------------------------
namespace ratel
{
#if defined(_MSC_VER)
#define STRUCT_PACK_BEGIN __pragma(pack(push, 1))
#define STRUCT_PACK_END __pragma(pack(pop))
#elif defined(__MINGW32__) || defined(__GNUC__)
#define STRUCT_PACK_BEGIN _Pragma("pack(push, 1)")
#define STRUCT_PACK_END _Pragma("pack(pop)")
#else
#error "Unsupported platform"
#endif
//-----------------------------------------------------------------------------
uint16_t htons(uint16_t val);
uint32_t htonl(uint32_t val);
//-----------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------
#endif // RatelInetUtilsH
