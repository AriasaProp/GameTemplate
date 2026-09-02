#ifndef __GRAPHICS_INCLUDED__
#define __GRAPHICS_INCLUDED__

#include "math/vector.h"
#include "math/color.h"
#include "math/mat4.h"
#define MAX_ASSET_READING 256
#define MAX_UI_DRAW       200

enum {
  GRAPHICS_CLEAR_COLOR = 1,
  GRAPHICS_CLEAR_DEPTH = 2,
  GRAPHICS_CLEAR_STENCIL = 4,
};

typedef ushrt texture; // 16bit uint as texture index
typedef ushrt mesh;    // use 16bit uint as mesh index
typedef ushrt mesh_index;
typedef struct {
  vec2 pos, uv;
} flat_vertex;
typedef struct {
  vec3 pos;
  rgba_u c;
} mesh_vertex;


// graphics external function
extern void graphics_clear(const int);
extern void graphics_clearColor(const rgbaf);
extern texture graphics_genTexture(const uvec2, void*);
extern void graphics_bindTexture(const texture);
extern void graphics_setTextureParam(const int, const int);
extern void graphics_deleteTexture(const texture);
extern void graphics_flatRender(const texture, flat_vertex*, const iter);
extern mesh graphics_genMesh(mesh_vertex*, const iter, mesh_index*, const iter);
extern void graphics_setMeshTransform(const mesh, const mat4);
extern void graphics_meshRender(mesh*, const iter);
extern void graphics_deleteMesh(const mesh);

#endif // __GRAPHICS_INCLUDED__