#ifndef __GRAPHICS__
#define __GRAPHICS__

#include "common.h"

extern void graphics_initial();
extern bool graphics_validate();
extern void graphics_resize(bool,uint*);
extern void graphics_postRender();
extern void graphics_invalidate();
extern void graphics_destroy();

#endif // __GRAPHICS__