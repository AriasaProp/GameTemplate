#include "graphics.h"
#include <<dlfcn.h>

static void *api = NULL;
static ANativeWindow *window = NULL;

typedef bool(*validate_funct)(ANativeWindow*);
typedef void(*resize_funct)(bool,float*);
typedef void(*postRender_funct)(void);
typedef void(*invalidate_funct)(void);
typedef void(*destroy_funct)(void);

typedef vec2(*getScreen_funct)(void);
typedef vec2(*toScreen_funct)(const vec2);
typedef void(*clear_funct)(const int);
typedef void(*clearColor_funct)(const rgbaf);
typedef texture(*genTexture_funct)(const uvec2, void*);
typedef void(*bindTexture_funct)(const texture);
typedef void(*setTextureParam_funct)(const int, const int);
typedef void(*deleteTexture_funct)(const texture);
typedef void(*flatRender_funct)(const texture, flat_vertex*, const iter);
typedef mesh(*genMesh_funct)(mesh_vertex*, const iter, mesh_index*, const iter);
typedef void(*setMeshTransform_funct)(const mesh, const mat4);
typedef void(*meshRender_funct)(mesh*, const iter);
typedef void(*deleteMesh_funct)(const mesh);

static struct {
  validate_funct validate;
  resize_funct resize;
  postRender_funct postRender;
  invalidate_funct invalidate;
  destroy_funct destroy;
  
  
  getScreen_funct getScreen;
  toScreen_funct toScreen;
  clear_funct clear;
  clearColor_funct clearColor;
  genTexture_funct genTexture;
  bindTexture_funct bindTexture;
  setTextureParam_funct setTextureParam;
  flatRender_funct flatRender;
  deleteTexture_funct deleteTexture;
  genMesh_funct genMesh;
  setMeshTransform_funct setMeshTransform;
  meshRender_funct meshRender;
  deleteMesh_funct deleteMesh;
  
} engine = {0};

void androidGraphics_initial(void) {
  
}
void androidGraphics_attach(ANativeWindow *w) {
  window = w;
}
bool androidGraphics_validate(void) {
  // choice
  while (!api) {
    api = dlopen("libext_gles.so",RTLD_NOW);
    if (!api) {
      LOGE("failed to load libext_gles.so");
      return false;
    }
    engine.validate   = CAST(validate_funct)dlsym(api, "opengles_validate");
    engine.resize     = CAST(resize_funct)dlsym(api, "opengles_resize");
    engine.postRender = CAST(postRender_funct)dlsym(api, "opengles_postRender");
    engine.invalidate = CAST(invalidate_funct)dlsym(api, "opengles_invalidate");
    engine.destroy    = CAST(destroy_funct)dlsym(api, "opengles_destroy");
    // graphics 
    engine.getScreen        = CAST(getScreen_funct)dlsym(api, "opengles_getScreen");
    engine.toScreen         = CAST(toScreen_funct)dlsym(api, "opengles_toScreen");
    engine.clear            = CAST(clear_funct)dlsym(api, "opengles_clear");
    engine.clearColor       = CAST(clearColor_funct)dlsym(api, "opengles_clearColor");
    engine.genTexture       = CAST(genTexture_funct)dlsym(api, "opengles_genTexture");
    engine.bindTexture      = CAST(bindTexture_funct)dlsym(api, "opengles_bindTexture");
    engine.setTextureParam  = CAST(setTextureParam_funct)dlsym(api, "opengles_setTextureParam");
    engine.flatRender       = CAST(flatRender_funct)dlsym(api, "opengles_flatRender");
    engine.deleteTexture    = CAST(deleteTexture_funct)dlsym(api, "opengles_deleteTexture");
    engine.genMesh          = CAST(genMesh_funct)dlsym(api, "opengles_genMesh");
    engine.setMeshTransform = CAST(setMeshTransform_funct)dlsym(api, "opengles_setMeshTransform");
    engine.meshRender       = CAST(meshRender_funct)dlsym(api, "opengles_meshRender");
    engine.deleteMesh       = CAST(deleteMesh_funct)dlsym(api, "opengles_deleteMesh");
  }
  return engine.validate(window);
}
void androidGraphics_resize(bool s, float *data) {
  if (api) engine.resize(s, data);
}
void androidGraphics_postRender(void) {
  if (api) engine.postRender();
}
void androidGraphics_invalidate(void) {
  if (api) engine.invalidate();
  window = NULL;
}
void androidGraphics_destroy(void) {
  if (!api) return;
  engine.destroy();
  dlclose(api);
  memset(&engine, 0, sizeof(engine));
  api = NULL;
}


vec2 graphics_getScreen(void) {
  if (api) engine.getScreen();
}
vec2 graphics_toScreen(const vec2 v) {
  return api ? engine.toScreen(v) : VEC2_ZERO;
}
void graphics_clear(const int flag) {
  if (api) engine.clear(flag);
}
void graphics_clearColor(const rgbaf color) {
  if (api) engine.clearColor(color);
}
texture graphics_genTexture(const uvec2 size, void *bitmap) {
  return api ? engine.genTexture(size, bitmap) : 0;
}
void graphics_bindTexture(const texture t) {
  if (api) engine.bindTexture(t);
}
void graphics_setTextureParam(const int x, const int y) {
  if (api) engine.setTextureParam(x, y);
}
void graphics_deleteTexture(const texture t);
  if (api) engine.deleteTexture(t);
}
void graphics_flatRender(const texture t, flat_vertex *v, const iter i) {
  if (api) engine.flatRender(t,v,i);
}
mesh graphics_genMesh(mesh_vertex *v, const iter vi, mesh_index *i, const iter ii) {
  return api ? engine.genMesh(v,vi,ii,i) : 0;
}
void graphics_setMeshTransform(const mesh m, const mat4 mat) {
  if (api) engine.setMeshTransform(m, mat);
}
void graphics_meshRender(mesh *ms, const iter n) {
  if (api) engine.meshRender(ms, n);
}
void graphics_deleteMesh(const mesh m) {
  if (api) engine.deleteMesh(m);
}


