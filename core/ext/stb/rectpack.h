#ifndef _STB_RECTPACK_INCLUDED_
#define _STB_RECTPACK_INCLUDED_

#define STBRP__MAXVAL 0x7fffffff
typedef int stbrp_coord;
typedef struct {
  int id;
  stbrp_coord w, h, x, y;
} stbrp_rect;

#ifdef __cplusplus
extern  "C" {
#endif // __cplusplus

int stbrp_pack_rects(stbrp_rect *, int, int, int);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _STB_RECTPACK_INCLUDED_