/*
 *  Util Header
 *
 *  Provide basic function and constant for all source code
 *
 *
 *
 */

#ifndef _COMMON_INCLUDED_
#define _COMMON_INCLUDED_

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// ================================
//  Global Macro
// ================================

// clang/gcc builtin feature
#ifndef __has_builtin
  // non clang/gcc always false
  #define __has_builtin(x) 0
#endif

#if defined(_MSC_VER) && (_MSC_VER < 1600)
  typedef __int8 int8_t;
  typedef __int16 int16_t;
  typedef __int32 int32_t;
  typedef __int64 int64_t;
  typedef unsigned __int8 uint8_t;
  typedef unsigned __int16 uint16_t;
  typedef unsigned __int32 uint32_t;
  typedef unsigned __int64 uint64_t;
  typedef unsigned __int64 size_t;
#else
  #include <stdint.h>
#endif

#if defined(_MSC_VER)
  #define ROTL(X,Y)    _rotl(X,Y)
  #define INLINE        __forceinline
  #define CDECL         __cdecl
  #define UNUSED(x)     ((void)x)
  #define UNUSED_ARG(x) __pragma(warning(suppress : 4100 4101)) x
// todo: best cast on MSC?
  #define CAST(T) (T)
#elif defined(__GNUC__) || defined(__clang__)
  #if __has_builtin(__builtin_rotateleft32)
    #define ROTL(X,Y)    __builtin_rotateleft32(X, Y)
  #else
    #define ROTL(X,Y)    ((X << Y) | (X >> (~Y & -31)))
  #endif
  
  #define INLINE        inline
  #define CDECL         __attribute__((cdecl))
  #define UNUSED(x)     ((void)x)
  #define UNUSED_ARG(x) __attribute__((unused)) x

  #define CAST(T)
#else
  #error "unknown Compiler action"
#endif

#ifdef __cplusplus
  #define IS_ERROR(x) if (x) [unlikely]
  #define LIKELY(x)   likely(x)
  #define UNLIKELY(x) unlikely(x)
#elif __has_builtin(__builtin_expect)
  #define IS_ERROR(x) if (__builtin_expect(!!(x), 0))
  #define LIKELY(x)   __builtin_expect(!!(x), 1)
  #define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
  #define IS_ERROR(x) if (!!(x))
  #define LIKELY(x)   (x)
  #define UNLIKELY(x) (x)
#endif

#include <assert.h>
#define ASSERT(X) assert(X)

#define MAX(X, Y)      ((X > Y) ? (X) : (Y))
#define MIN(X, Y)      ((X < Y) ? (X) : (Y))
#define CLAMP(X, Y, Z) ((X < Z) ? ((Y > Z) ? (Z) : (Y)) : (X))

// ================================
//  Structure
// ================================

typedef struct {
  float r, g, b, a;
} fcolor; // hex color
typedef union {
  struct {
    uint8_t r, g, b, a;
  } u8;
  struct {
    uint16_t l, a;
  } u16;
  uint32_t u32;
} icolor; // packed color bit

// ===============================
//  Function
// ===============================

// helper

#ifndef COMMON_IMPLEMENTATION_
  #ifdef _WIN32
    extern __declspec(dllimport) int __stdcall MultiByteToWideChar(unsigned int, unsigned long, const char *, int, wchar_t *, int);
    extern __declspec(dllimport) int __stdcall WideCharToMultiByte(unsigned int, unsigned long, const wchar_t *, int, char *, int, const char *, int *);
    extern int convert_wchar_to_utf8(char *, size_t, const wchar_t *);
  #endif
  extern void flipBytes(uint8_t *, const size_t);
  
#endif // COMMON_IMPLEMENTATION_


// dynamic array macro
#define DA_RESERV(da, s) do { \
  if((da)->cap >= (s)) break; \
  (da)->cap = ((s) & ~7) + 8 * !!((s) & 7); \
  (da)->items = CAST((da)->items)realloc((void*)(da)->items, (da)->cap * sizeof(*((da)->items))); \
  ASSERT((da)->items);\
} while (0)

#define DA_APPEND(da, i) do { \
  DA_RESERV(da, (da)->count + 1); \
  (da)->items[(da)->count++] = (i); \
} while(0)

#define DA_UNORDER_REMOVE(da,i) (da)->items[ASSERT(i < (da)->count), --(da)->count] = (da)->items[i]
#define DA_POP(da) (da)->items[ASSERT((da)->count), --(da)->count] 
#define DA_LAST(da) (da)->items[ASSERT((da)->count), (da)->count - 1] 
#define DA_CLEAN(da) (da)->count = 0;

#define DA_FREE(da) do {  \
  if (!(da)->cap) break;  \
  free((da)->items);      \
  memset(da,0,sizeof(*(da)));  \
} while (0)

#endif // _COMMON_INCLUDED_
