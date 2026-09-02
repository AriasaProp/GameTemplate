#include "common.h"
#include <math.h>

#include <byteswap.h>
#include <time.h>

#if defined(_WIN32) || defined(_WIN64)
#  pragma intrinsic(__rdtsc)
// win32/64 wide character support
int convert_wchar_to_utf8(char *buffer, iter bufferlen, const wchar_t *input) {
  return WideCharToMultiByte(65001 /* UTF8 */, 0, input, -1, buffer, (int)bufferlen, NULL, NULL);
}
#endif

#define NAIVE_FLIP(X,L,R) (((X) << (L % (sizeof(X) * 8))) | ((X) >> (R % (sizeof(X) * 8))))
ubyte imath_flip8(ubyte x) {
  return CAST(ubyte)NAIVE_FLIP(x,4,4);
}
ushrt imath_flip16(ushrt x) {
#if BLTN(__builtin_bswap16)
  return __builtin_bswap16(x);
#elif defined(__GLIBC__)
  return bswap_16(x);
#else
	return NAIVE_FLIP(x,8,8);
#endif
} 
uint32 imath_flip32(uint32 x) {
#if BLTN(__builtin_bswap32)
  return __builtin_bswap32(x);
#elif defined(__GLIBC__)
  return bswap_32(x);
#else
	util_memflip(&x, sizeof(uint32));
	return x;
#endif
}
uint64 imath_flip64(uint64 x) {
#if BLTN(__builtin_bswap64)
  return __builtin_bswap64(x);
#elif defined(__GLIBC__)
  return bswap_64(x);
#else
	util_memflip(&x, sizeof(uint64));
	return x;
#endif
}
int32 imath_rotl32(int32 x, const iter n) {
#if BLTN(__builtin_rotateleft32)
  return __builtin_rotateleft32(x, n);
#elif defined(_MSC_VER)
  return _rotl(x, n);
#else
  return CAST(int32)NAIVE_FLIP(x,n,(-n&31));
#endif
}
int64 imath_rotl64(int64 x, const iter n) {
#if BLTN(__builtin_rotateleft64)
  return __builtin_rotateleft64(x, n);
#elif defined(_MSC_VER)
  return _rotl64(x, n);
#else
  return CAST(int64)NAIVE_FLIP(x,n,(-n&63));
#endif
}
int32 imath_rotr32(int32 x, const iter n) {
#if BLTN(__builtin_rotateright32)
  return __builtin_rotateright32(x, n);
#elif defined(_MSC_VER)
  return _rotr(x, n);
#else
  return CAST(int32)NAIVE_FLIP(x,(-n&31),n);
#endif
}
int64 imath_rotr64(int64 x, const iter n) {
#if BLTN(__builtin_rotateright64)
  return __builtin_rotateright64(x, n);
#elif defined(_MSC_VER)
  return _rotr64(x, n);
#else
  return CAST(int64)NAIVE_FLIP(x,(-n&63),n);
#endif
}