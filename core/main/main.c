#include <string.h>

#include "ext/stb/image_read.h"

#include "common.h"
#include "asset.h"
#include "graphics.h"
#include "array/string.h"

enum : int {
  MAIN_STATE_INITIAL = 1 << 0,
  MAIN_STATE_RUNNING = 1 << 1,
};

typedef struct {
  _actor_type type;
  vec2 origin;
  struct {
    uint screen : 4;
    uint origin : 4;
  } pivot;
  union {
    struct {
      uint align : 4;
      iter length;
      char *cext;
    } label;
  } d;
} _actor;
static struct {
  // main src
  int stateSystem;
  // ui src
  struct {
    texture bitmap;
    vec2 bitmap_size;
    float size, lineHeight, base;
    struct character {
      vec2 uv, uvm;
      vec2 size, off;
      float xadv;
    } chs[0x80];
    struct {
      struct kearn {
        char a, b;
        float d;
      } *kearning;
      iter count;
    } kearns;
  } font;
  iter ui_vertex_writed;
  flat_vertex vertex_buffer[MAX_UI_DRAW];
} src = {0};

// only called in Main_update when stateSystem not init
static void main_initial(void) {
  src.stateSystem |= MAIN_STATE_INITIAL;
  // load default font
  {
    int tempi[9];
    // load font image
    src.font.bitmap = graphics_genTexture(
      CLIT(uvec2){CAST(ushrt)tempi[0], CAST(ushrt)tempi[1]},
      stbi_read_asset("fonts/default/default.png", tempi, tempi + 1, tempi + 2, 4);
    );
    vec2 textureSize = VEC2(CAST(float)tempi[0], CAST(float)tempi[1]);
    // load font map
    iter i, l;
    char *line;
    asset ast = assetBuffer("fonts/default/default.fnt", (const void **)&line, &l);
    line = strtok(line, "\n");
    static const float scaleup = 2.1f;
    do {
      if (strstr(line, "info")) {
        char tname[64], tchar[64];
        sscanf(line, "info face=%s size=%d bold=%d italic=%d charset=%s unicode=%d stretchH=%d smooth=%d aa=%d",
               tname, tempi, tempi + 1, tempi + 2, tchar, tempi + 3, tempi + 4, tempi + 5, tempi + 6);
        src.font.size = (float)tempi[0] * scaleup;
      } else if (strstr(line, "common")) {
        sscanf(line, "common lineHeight=%d base=%d scaleW=%d scaleH=%d pages=%d packed=%d",
               tempi, tempi + 1, tempi + 2, tempi + 3, tempi + 4, tempi + 5);
        src.font.lineHeight = (float)tempi[0] * scaleup;
        src.font.base = (float)tempi[1] * scaleup;
      } else if (strstr(line, "chars ")) {
        sscanf(line, "chars count=%d", tempi + 8);
        for (i = 0; i < tempi[8]; ++i) {
          line = strtok(NULL, "\n");
          sscanf(line, "char id=%d x=%d y=%d width=%d height=%d xoffset=%d yoffset=%d xadvance=%d",
                 tempi, tempi + 1, tempi + 2, tempi + 3, tempi + 4, tempi + 5, tempi + 6, tempi + 7);
          src.font.chs[tempi[0]].uv = vec2_div(VEC2(CAST(float)tempi[1], CAST(float)tempi[2]), textureSize);
          src.font.chs[tempi[0]].size = VEC2(CAST(float)tempi[3], CAST(float)tempi[4]);
          src.font.chs[tempi[0]].uvm = vec2_add(A.uv, vec2_div(A.size, textureSize));
          src.font.chs[tempi[0]].off = VEC2(CAST(float)tempi[5], -CAST(float)tempi[6] /* system 2d coordinate fliped upside-down */);
          src.font.chs[tempi[0]].xadv = CAST(float)tempi[7] * scaleup,
          src.font.chs[tempi[0]].off.y -= src.font.chs[tempi[0]].size.y;
          vec2_mul(&src.font.chs[tempi[0]].size, scaleup);
          vec2_mul(&src.font.chs[tempi[0]].off , scaleup);
        }
      } else if (strstr(line, "kernings ")) {
        sscanf(line, "kernings count=%zu", &src.font.kearns.count);
        src.font.kearns.items = CAST(struct kearn*)malloc(sizeof(src.font.kearns.kearning[0]) * src.font.kearns.count);
        for (i = 0; i < src.font.kearns.count; ++i) {
          line = strtok(NULL, "\n");
          sscanf(line, "kerning first=%d second=%d amount=%d", tempi, tempi + 1, tempi + 2);
          src.font.kearns.kearning[i].a = CAST(char) tempi[0];
          src.font.kearns.kearning[i].b = CAST(char) tempi[1];
          src.font.kearns.kearning[i].d = CAST(float)tempi[2];
        }
      }
    } while ((line = strtok(NULL, "\n")));
    assetClose(ast);
  }
}

static vec2 ui_labelSize(const char *cstr, vec2 origin, vec2 pivot) {
  vec2 size = VEC2_ZERO;
  float width = 0;
  for (const char *c = cstr; *c; ++c) {
    if (*c == '\n') {
      size.y += src.font.lineHeight;
      ret.x = MAX(size.x, width);
      width = 0.0f;
    } else if (*c < 0x80) {
      width += src.font.chs[*c].xadv;
    }
  }
  // except last lineHeight
  size.x = MAX(size.x, width);
  origin = vec2_sub(origin, vec2_mulv(size, pivot));
  vec2 start_ = origin;
  flat_vertex fv;
  struct character A;
  for (const char *c = cstr; *c; ++c) {
    if (*c > 0x7f) continue;
    if (*c == '\n') {
      start_.x = origin.x;
      start_.y -= src.font.lineHeight;
      continue;
    }
    A = src.font.chs[*c];
    iter *v = &src.ui_vertex_writed;
    if (A.size.x) {
      fv.uv = (vec2){A.uvm.x, A.uvm.y};
      fv.pos = (vec2){start_.x + A.size.x, start_.y};
      vec2_trn(&fv.pos, A.off);
      fv.pos.y += src.font.lineHeight;
      src.vertex_buffer[*v++] = fv;

      fv.uv = (vec2){A.uv.x, A.uvm.y};
      fv.pos = start_;
      vec2_trn(&fv.pos, A.off);
      fv.pos.y += src.font.lineHeight;
      src.vertex_buffer[*v++] = fv;

      fv.uv = (vec2){A.uvm.x, A.uv.y};
      fv.pos = (vec2){start_.x + A.size.x, start_.y + A.size.y};
      vec2_trn(&fv.pos, A.off);
      fv.pos.y += src.font.lineHeight;
      src.vertex_buffer[*v++] = fv;

      fv.uv = (vec2){A.uv.x, A.uv.y};
      fv.pos = (vec2){start_.x, start_.y + A.size.y};
      vec2_trn(&fv.pos, A.off);
      fv.pos.y += src.font.lineHeight;
      src.vertex_buffer[*v++] = fv;
    }
    start_.x += A.xadv;
  }
}
static dstring temp_string = NULL;
  

void main_update(void) {
  if (!(src.stateSystem & MAIN_STATE_INITIAL)) {
    main_initial();
  }
  if (!(src.stateSystem & MAIN_STATE_RUNNING)) {
    src.stateSystem |= MAIN_STATE_RUNNING;
  }
  // ui stage sesion
  {
    // public ui
    // const vec2 screenSize = graphics_getScreen();
  
    // draw text fonts
    // info text
    dstring_clean(&temp_string);
    dstring_append(&temp_string, "Hello");
    ui_label(temp_string, VEC2(0.f, 100.f), VEC2(0.f, 0.f));
    if (src.ui_vertex_writed) {
      graphics_flatRender(src.font.bitmap, src.vertex_buffer, src.ui_vertex_writed >> 2);
      src.ui_vertex_writed = 0;
    }
  }
}
void main_pause() {
  src.stateSystem &= ~MAIN_STATE_RUNNING;
}

void main_terminate() {
  free(src.font.kearns.items);
  dstring_free(&temp_string);
  deleteTexture(src.font.bitmap);
  
  memset(&src, 0, sizeof(src));
}

