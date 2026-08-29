/* *****************************************************************************
 * common.h v0.0.0000
 * 
 * Provide basic function and constant for all source code
 * 
 * 
 * 
 * *****************************************************************************/

#ifndef _COMMON_INCLUDED_
#define _COMMON_INCLUDED_

#include <limits.h> // INT_MAX
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>

// ================================
//  Global Macro & Primitive Redefinition
// ================================
#if defined(__ARM_NEON) || defined(__NEON__)
#  include <arm_neon.h>
#endif
#ifdef _MSC_VER
#  include <intrin.h>
#endif
#if defined(__AVX__) || defined(__AVX2__) || defined(__AVX512F__)
#  include <immintrin.h>    // AVX and later
#endif
#ifdef __SSE4_2__
#  include <nmmintrin.h>    // SSE 4.2
#endif
#ifdef __SSE4_1__
#  include <smmintrin.h>    // SSE 4.1
#endif
#ifdef __SSSE3__
#  include <tmmintrin.h>    // SSSE3
#endif
#ifdef __SSE3__
#  include <pmmintrin.h>    // SSE3
#endif
#ifdef __SSE2__
#  include <emmintrin.h>    // SSE2
#endif
#ifdef __SSE__
#  include <xmmintrin.h>    // SSE
#endif


#if (defined(_MSC_VER) && _MSC_VER < 1600) /*|| defined(__SYMBIAN32__) */
  typedef          __int8   byte;
  typedef          __int16  shrt;
  typedef          __int32  int32;
  typedef          __int64  int64;
  typedef unsigned __int8 	ubyte;
  typedef unsigned __int16 	ushrt;
  typedef unsigned __int32 	uint32;
  typedef unsigned __int64 	uint64;
  #ifdef __SIZEOF_INT128__
  typedef          __int128  int128;
  typedef unsigned __int128  uint128;
	#endif
  typedef unsigned __int64 	iter;
  
#else
  #include <stdint.h>
  
  typedef int8_t    byte;
  typedef int16_t   shrt;
  typedef int32_t   int32;
  typedef int64_t   int64;
  typedef uint8_t 	ubyte;
  typedef uint16_t 	ushrt;
  typedef uint32_t 	uint32;
  typedef uint64_t 	uint64;
  #ifdef __SIZEOF_INT128__
  typedef __int128_t 	  int128;
  typedef __uint128_t 	uint128;
	#endif
  typedef size_t    iter;
#endif

typedef long long          llong;
typedef unsigned int       uint;
typedef unsigned long      ulong;
typedef unsigned long long ullong;

#define ASSERT(X)             assert(X)
#define TODO(X)               // Message that need todo in future: (X)
#define PRIVATE_STRINGIFY(X)  #X
#define STRINGIFY(X)          PRIVATE_STRINGIFY(X)
#define STACK_ARR_LEN(X)      (sizeof((X)) / sizeof((X)[0]))

#ifdef _MSC_VER
  #if defined(_WIN32) || defined(WIN32)
    #ifndef _CRT_SECURE_NO_WARNINGS
    #  define _CRT_SECURE_NO_WARNINGS
    #endif
    #ifndef _CRT_NONSTDC_NO_DEPRECATE
    #  define _CRT_NONSTDC_NO_DEPRECATE
    #endif
  #endif

#  define CDECL            __cdecl
#  define UNUSED(x)        ((void)x)
#  define UNUSED_ARG(x)    __pragma(warning(suppress : 4100 4101)) x
#  define NONNULL_ARG(x)   __attribute__((nonnull)) x
#  define BLTN(x)          0
	#define SIMD_ALIGN(type, name) __declspec(align(16)) type name
#  define ALIGN(N)     __declspec(align(N))
#  define PACKED(type) __pragma(pack(push, 1)) type __pragma(pack(pop))
#elif defined(__GNUC__) || defined(__clang__)
#  define CDECL            /* no translate */
#  define UNUSED(x)        ((void)x)
#  define UNUSED_ARG(x)    __attribute__((unused)) x
#  define NONNULL_ARG(x)   __attribute__((nonnull)) x
#  define BLTN(x)          __has_builtin(x)
#  define SIMD_ALIGN(type, name) type name __attribute__((aligned(16)))
#  define ALIGN(N)     __attribute__((aligned(N)))
#  define PACKED(type) __attribute__((packed)) type
#else /* Unknown compiler */
  #error "Not ready for this compiler"
#endif

// thread local
#if defined(__cplusplus) &&  __cplusplus >= 201103L
#  define THREAD_LOCAL       thread_local
#elif defined(__GNUC__) && __GNUC__ < 5
#  define THREAD_LOCAL       __thread
#elif defined(_MSC_VER)
#  define THREAD_LOCAL       __declspec(thread)
#elif defined (__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#  define THREAD_LOCAL       _Thread_local
#elif defined(__GNUC__)
#  define THREAD_LOCAL       __thread
#else
#  define THREAD_LOCAL
#endif


#define MIN(X,Y)  (((X) < (Y)) ? (X) : (Y))
#define MAX(X,Y)  (((X) > (Y)) ? (X) : (Y))
#define CLAMP(min, X, max)  MAX(min, MIN(max, X))
#define BETWEEN(min, X, max)  ((X) >= (min) && (X) <= (max))

#ifdef __cplusplus
#  define IS_ERROR(x) if (!!(x)) [[unlikely]]
#  define LIKELY(x)   (x) [[likely]]
#  define UNLIKELY(x) (x) [[unlikely]]
#  define EXPECT(x,y) ((x) == (y)) [[likely]]
  
#  define CLIT(T) T
#  define CAST(T) (decltype(T))
extern "C" {
#else
#  if BLTN(__builtin_expect)
#    define IS_ERROR(x) if (__builtin_expect(!!(x), 0))
#    define LIKELY(x)   __builtin_expect(!!(x), 1)
#    define UNLIKELY(x) __builtin_expect(!!(x), 0)
#    define EXPECT(x,y) __builtin_expect((x), (y))
#  else
#    define IS_ERROR(x) if (!!(x))
#    define LIKELY(x)   (x)
#    define UNLIKELY(x) (x)
#    define EXPECT(x,y) ((x) == (y))
#  endif
#  define CLIT(T) (T)
#  define CAST(T) (T)
#endif // __cplusplus


#ifdef _WIN32
int convert_wchar_to_utf8(char *, iter, const wchar_t *);
#endif // _WIN32

#ifdef __cplusplus
}
#endif

#endif // _COMMON_INCLUDED_
