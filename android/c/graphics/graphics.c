#include <android/native_activity.h>

#include "graphics/graphics.h"

static void *api = NULL;

typedef bool(*validate_funct)(ANativeWindow*);
typedef void(*resize_funct)(bool,uint*);
typedef void(*postRender_funct)(void);
typedef void(*invalidate_funct)(void);
typedef void(*destroy_funct)(void);

static validate_funct   engine_validate   = NULL;
static resize_funct     engine_resize     = NULL;
static postRender_funct engine_postRender = NULL;
static invalidate_funct engine_invalidate = NULL;
static destroy_funct    engine_destroy    = NULL;

void graphics_initial() {
  
}
bool graphics_validate(ANativeWindow *window) {
  // choice
  while (!api) {
    api = dlopen("libext_gles.so",RTLD_NOW);
    if (!api) {
      LOGE("failed to load libext_gles.so");
      return false;
    }
    engine_validate   = CAST(validate_funct)  dlsym(api, "opengles_validate"  );
    engine_resize     = CAST(resize_funct)    dlsym(api, "opengles_resize"    );
    engine_postRender = CAST(postRender_funct)dlsym(api, "opengles_postRender");
    engine_invalidate = CAST(invalidate_funct)dlsym(api, "opengles_invalidate");
    engine_destroy    = CAST(dedtroy_funct)   dlsym(api, "opengles_destroy"   );
  }
  return engine_validate(window);
}
void graphics_resize(bool s, uint *data) {
  if (api)
    engine_resize(s, data);
}
void graphics_postRender() {
  if (api)
    engine_postRender();
}
void graphics_invalidate() {
  if (api)
    engine_invalidate();
}
void graphics_destroy() {
  if (api) {
    engine_destroy();
    dlclose(api);
  }
  api = NULL;
  engine_validate   =  NULL; 
  engine_resize     =  NULL; 
  engine_postRender =  NULL;
  engine_invalidate =  NULL;
  engine_destroy    =  NULL; 
}

