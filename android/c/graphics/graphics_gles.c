#include "common.h"
#include "assets.h"
#include "graphics.h"
#include "math/color.h"
#include "math/vector.h"
#include "math/mat4.h"
#include "log.h"

#include <android/native_window.h>
#include <dlfcn.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <GLES3/gl32.h>
#include <EGL/egl.h>

#define MAX_MSG                512
static GLint success;
static GLchar msg[MAX_MSG];

static void getErrorGL(const char *X) {
  static GLenum error;
  while ((error = glGetError()))
    LOGE("Err %s 0x%x\n", X, error);
}
#define check(X) do { \
  X; \
  getErrorGL(#X); \
} while (0)
#define MAX_RESOURCE 256
// mesh flags for uniform update
enum {
  MESH_VERTEX_DIRTY = 1,
  MESH_INDEX_DIRTY = 2,
};
// flags global 2d/3d uniform update
enum {
  UI_UPDATE      = 1 << 0,
  WORLD_UPDATE   = 1 << 1,
  RESIZE_DISPLAY = 1 << 2,
  RESIZE_ONLY    = 1 << 3,
};
// private function
enum {
  TERM_EGL_SURFACE = 1,
  TERM_EGL_CONTEXT = 2,
  TERM_EGL_DISPLAY = 4,
};
typedef struct {
  GLuint id;
  uvec2 size;
  void *data;
} opengles_texture;
typedef struct {
  GLuint vao, vbo, ibo;
  int flags;
  iter vertex_len, index_len;
  mesh_vertex *vertexs;
  mesh_index *indices;
  mat4 trans;
} opengles_mesh;
static struct androidGraphics {
  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;
  EGLConfig eConfig;
  int flags;

  struct {
    GLint shader, uniform_proj, uniform_tex;
    GLuint vao, vbo, ibo;
  } ui;
  struct {
    GLint shader, uniform_proj, uniform_transProj;
  } world;

  vec2 viewportSize; //
  vec2 screenSize;   //
  vec4 insets;

  opengles_texture textures[MAX_RESOURCE];
  opengles_mesh meshes[MAX_RESOURCE];
} src = {0};

// core implementation
vec2 opengles_getScreen(void) { return src.screenSize; }
vec2 opengles_toScreen(const vec2 v) {
  return CLIT(vec2) {
    v.x - src.insets.x,
    src.viewportSize.y - v.y - src.insets.w
  };
}

void opengles_clear(const int m) {
  check(glClear(
    (((m & GRAPHICS_CLEAR_COLOR) == GRAPHICS_CLEAR_COLOR) * GL_COLOR_BUFFER_BIT) |
    (((m & GRAPHICS_CLEAR_DEPTH) == GRAPHICS_CLEAR_DEPTH) * GL_DEPTH_BUFFER_BIT) |
    (((m & GRAPHICS_CLEAR_STENCIL) == GRAPHICS_CLEAR_STENCIL) * GL_STENCIL_BUFFER_BIT)));
}
void opengles_clearColor(const rgbaf c) {
  check(glClearColor(c.r, c.g, c.b, c.a));
}
texture opengles_genTexture(const uvec2 size, void *data) {
  texture i = 1;
  while (i < MAX_RESOURCE) {
    if (src.textures[i].size.x == 0)
      break;
    ++i;
  }
  if (i >= MAX_RESOURCE)
    return 0; // reach limit texture total so return default texture
  src.textures[i].size = size;
  src.textures[i].data = data;
  check(glGenTextures(1, &src.textures[i].id));
  check(glBindTexture(GL_TEXTURE_2D, src.textures[i].id));
  check(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
  check(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
  check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
  check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
  check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
  check(glBindTexture(GL_TEXTURE_2D, 0));
  return i;
}
void opengles_bindTexture(const texture t) {
  check(glBindTexture(GL_TEXTURE_2D, src.textures[t].id));
}
void opengles_setTextureParam(const int param, const int val) {
  check(glTexParameteri(GL_TEXTURE_2D, param, val));
}
void opengles_deleteTexture(const texture t) {
  check(glDeleteTextures(1, &src.textures[t].id));
  free(src.textures[t].data);
  memset((void *)(src.textures + t), 0, sizeof(opengles_texture));
}
void opengles_flatRender(const texture t, flat_vertex *v, const iter l) {
  check(glDisable(GL_DEPTH_TEST));
  check(glUseProgram(src.ui.shader));
  if (src.flags & UI_UPDATE) {
    check(glUniformMatrix4fv(src.ui.uniform_proj, 1, GL_FALSE, CLIT(float[]){
      2.f / src.viewportSize.x, 0.f, 0.f, 0.f,
      0.f, 2.f / src.viewportSize.y, 0.f, 0.f,
      0.f, 0.f, 1.f, 0.f,
        (2.0f * src.insets.x / src.viewportSize.x) - 1.0f,
        (2.0f * src.insets.w / src.viewportSize.y) - 1.0f,
        0.f, 1.f,
    }));
    src.flags &= ~UI_UPDATE;
  }
  check(glActiveTexture(GL_TEXTURE0));
  check(glBindTexture(GL_TEXTURE_2D, src.textures[t].id));
  check(glUniform1i(src.ui.uniform_tex, 0));
  check(glBindVertexArray(src.ui.vao));
  check(glBindBuffer(GL_ARRAY_BUFFER, src.ui.vbo));
  check(glBufferSubData(GL_ARRAY_BUFFER, 0, 4 * l * sizeof(flat_vertex), (void *)v));
  check(glDrawElements(GL_TRIANGLES, 6 * l, GL_UNSIGNED_SHORT, NULL));
  check(glBindVertexArray(0));
  check(glBindTexture(GL_TEXTURE_2D, 0));
  check(glUseProgram(0));
}
mesh opengles_genMesh(mesh_vertex *v, const iter vl, mesh_index *i, const iter il) {
  mesh m = 0;
  while (m < MAX_RESOURCE) {
    if (src.meshes[m].vertex_len == 0)
      break;
    ++m;
  }
  if (m >= MAX_RESOURCE)
    return -1; // reach limit mesh total so return invalid number
  src.meshes[m].vertex_len = vl;
  src.meshes[m].vertexs = v;
  src.meshes[m].index_len = il;
  src.meshes[m].indices = i;
  src.meshes[m].trans = MAT4_IDT;

  check(glGenVertexArrays(1, &src.meshes[m].vao));
  check(glGenBuffers(2, &src.meshes[m].vbo));
  check(glBindVertexArray(src.meshes[m].vao));
  check(glBindBuffer(GL_ARRAY_BUFFER, src.meshes[m].vbo));
  check(glBufferData(GL_ARRAY_BUFFER, vl * sizeof(mesh_vertex), (void *)v, GL_STATIC_DRAW));
  check(glEnableVertexAttribArray(0));
  check(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex), (void *)0));
  check(glEnableVertexAttribArray(1));
  check(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(mesh_vertex), (void *)sizeof(vec3)));
  check(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, src.meshes[m].ibo));
  check(glBufferData(GL_ELEMENT_ARRAY_BUFFER, il * sizeof(mesh_index), (void *)i, GL_STATIC_DRAW));
  check(glBindVertexArray(0));
  src.meshes[m].flags |= MESH_VERTEX_DIRTY | MESH_INDEX_DIRTY;
  return m;
}
void opengles_setMeshTransform(const mesh ms, const mat4 mat) {
  src.meshes[ms].trans = mat;
}
void opengles_meshRender(mesh *ms, const iter l) {
  check(glEnable(GL_DEPTH_TEST));
  check(glUseProgram(src.world.shader));
  if (src.flags & WORLD_UPDATE) {
    check(glUniformMatrix4fv(src.world.uniform_proj, 1, GL_FALSE, CLIT(float[]){
      2.f / src.viewportSize.x, 0.f, 0.f, 0.f,
      0.f, 2.f / src.viewportSize.y, 0.f, 0.f,
      0.f, 0.f, 1.f, 0.f,
      0.f, 0.f, 0.f, 1.f,
    }));
    src.flags &= ~WORLD_UPDATE;
  }
  for (iter i = 0; i < l; i++) {
    opengles_mesh m = src.meshes[ms[i]];
    check(glUniformMatrix4fv(src.world.uniform_transProj, 1, GL_FALSE, m.trans.v));
    check(glBindVertexArray(m.vao));
    if (m.flags & MESH_VERTEX_DIRTY) {
      check(glBindBuffer(GL_ARRAY_BUFFER, m.vbo));
      check(glBufferSubData(GL_ARRAY_BUFFER, 0, m.vertex_len * sizeof(mesh_vertex), (void *)m.vertexs));
      m.flags &= ~MESH_VERTEX_DIRTY;
    }
    if (m.flags & MESH_INDEX_DIRTY) {
      check(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ibo));
      check(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m.index_len * sizeof(mesh_index), (void *)m.indices));
      m.flags &= ~MESH_INDEX_DIRTY;
    }
    m.flags = 0;
    check(glDrawElements(GL_TRIANGLES, m.index_len, GL_UNSIGNED_SHORT, NULL));
  }
  check(glBindVertexArray(0));
  check(glUseProgram(0));
}
void opengles_deleteMesh(mesh m) {
  check(glDeleteVertexArrays(1, &src.meshes[m].vao));
  check(glDeleteBuffers(2, &src.meshes[m].vbo));
  free(src.meshes[m].vertexs);
  free(src.meshes[m].indices);
  memset(src.meshes + m, 0, sizeof(opengles_mesh));
}

static void killEGL(const int EGLTermReq) {
  if (!EGLTermReq || (src.display == EGL_NO_DISPLAY))
    return;
  if (src.textures[0].id) {
    // world draw
    check(glDeleteProgram(src.world.shader));
    // flat draw
    check(glDeleteProgram(src.ui.shader));
    check(glDeleteVertexArrays(1, &src.ui.vao));
    check(glDeleteBuffers(2, &src.ui.vbo));
    // mesh
    for (mesh i = 0; i < MAX_RESOURCE; ++i) {
      if (src.meshes[i].vertex_len == 0)
        continue;
      check(glDeleteVertexArrays(1, &src.meshes[i].vao));
      check(glDeleteBuffers(2, &src.meshes[i].vbo));
    }
    // texture
    for (texture i = 0; i < MAX_RESOURCE; ++i) {
      if (src.textures[i].size.x == 0)
        continue;
      check(glDeleteTextures(1, &src.textures[i].id));
      src.textures[i].id = 0;
    }
  }
  eglMakeCurrent(src.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  if (src.surface && (EGLTermReq & 5)) {
    // invalidate Framebuffer, RenderBuffer
    eglDestroySurface(src.display, src.surface);
    src.surface = EGL_NO_SURFACE;
  }
  if (src.context && (EGLTermReq & 6)) {
    // invalidating gles
    eglDestroyContext(src.display, src.context);
    src.context = EGL_NO_CONTEXT;
  }
  if (EGLTermReq & 4) {
    eglTerminate(src.display);
    src.display = EGL_NO_DISPLAY;
  }
}
// android purpose
void opengles_invalidate(void) {
  killEGL(TERM_EGL_SURFACE);
}
void opengles_resize(bool sys, vec4 ins) {
  src.flags |= sys ? RESIZE_DISPLAY : RESIZE_ONLY;
  src.insets = ins;
  src.screenSize.x = src.viewportSize.x - ins.x - ins.z;
  src.screenSize.y = src.viewportSize.y - ins.y - ins.w;
}
bool opengles_validate(ANativeWindow *window) {
  if (!window) return false;
  if ((src.display == EGL_NO_DISPLAY) || (src.context == EGL_NO_CONTEXT) || (src.surface == EGL_NO_SURFACE)) {
    if (src.display == EGL_NO_DISPLAY) {
      src.surface = EGL_NO_SURFACE;
      src.context = EGL_NO_CONTEXT;
      src.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
      if (src.display == EGL_NO_DISPLAY) {
        LOGW("Failed to get EGLDisplay");
        return false;
      }
      EGLint temp, temp1;
      eglInitialize(src.display, &temp, &temp1);
      if (temp < 1 || temp1 < 3) { // unsupported egl version lower than 1.3
        LOGW("EGL version is below 1.3");
        return false;
      }
      const EGLint configAttr[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,
        EGL_BUFFER_SIZE, 16,
        EGL_NONE};
      eglChooseConfig(src.display, configAttr, NULL, 0, &temp);
      if (temp <= 0) {
        LOGW("There is no fit config for minimum EGLConfig attribute");
        return false;
      }
      EGLConfig *configs = (EGLConfig *)malloc(temp * sizeof(EGLConfig));
      eglChooseConfig(src.display, configAttr, configs, temp, &temp);
      iter i = 0, j = temp, k = 0, l;
      do {
        l = 1;
#define EGL_CONFIG_EVA(X)                                     \
  if (eglGetConfigAttrib(src.display, configs[i], X, &temp)) \
  l += temp
        EGL_CONFIG_EVA(EGL_BUFFER_SIZE);
        EGL_CONFIG_EVA(EGL_DEPTH_SIZE);
        EGL_CONFIG_EVA(EGL_STENCIL_SIZE);
        EGL_CONFIG_EVA(EGL_SAMPLES);
        // TODO: and more attributes
#undef EGL_CONFIG_EVA
        if (l > k) {
          k = l;
          src.eConfig = configs[i];
        }
      } while ((++i) < j);
      free(configs);
    }
    if (src.context == EGL_NO_CONTEXT) {
      src.context = eglCreateContext(src.display, src.eConfig, NULL, CLIT(EGLint[]){EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE});
      if (src.context == EGL_NO_CONTEXT) {
        LOGW("Failed to create EGLContext");
        return false;
      }
    }
    if ((src.surface == EGL_NO_SURFACE) &&
        ((src.surface = eglCreateWindowSurface(src.display, src.eConfig, window, NULL)) == EGL_NO_SURFACE)
       ) {
      LOGW("Failed to create EGLSurface");
      return false;
    }
    eglMakeCurrent(src.display, src.surface, src.surface, src.context);
    if (!src.textures[0].id) {

      // when validate, projection need to be update
      src.flags |= WORLD_UPDATE | UI_UPDATE;
      // set clear
      check(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
      // cullface to front
      check(glEnable(GL_CULL_FACE));
      check(glCullFace(GL_FRONT));
      // enable depth
      check(glDepthFunc(GL_LESS));
      check(glDepthRangef(0.0f, 1.0f));
      check(glClearDepthf(1.0f));
      // enable blend
      check(glEnable(GL_BLEND));
      check(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
      {
        const void *tempbuf;
        asset ast;
        iter tempbufl;
        GLuint vi, fi;
        // flat draw
#define checkLinkProgram(X) do { \
  glLinkProgram(X); \
  glGetProgramiv(X, GL_LINK_STATUS, &success); \
  if (!success) { \
    glGetProgramInfoLog(X, MAX_MSG, NULL, msg); \
    LOGE("Shader link: %s", msg); \
  } \
} while (0)
#define checkCompileShader(X) do { \
  glCompileShader(X); \
  glGetShaderiv(X, GL_COMPILE_STATUS, &success); \
  if (!success) { \
    glGetShaderInfoLog(X, MAX_MSG, NULL, msg); \
    LOGE("Shader compile: %s", msg); \
  } \
} while (0)
        {
          check(src.ui.shader = glCreateProgram());
          check(vi = glCreateShader(GL_VERTEX_SHADER));
          ast = assetBuffer("shaders/flatdraw.vert", &tempbuf, &tempbufl);
          check(glShaderSource(vi, 1, (const GLchar **)&tempbuf, (const GLint *)&tempbufl));
          assetClose(ast);
          checkCompileShader(vi);
          check(glAttachShader(src.ui.shader, vi));
          check(fi = glCreateShader(GL_FRAGMENT_SHADER));
          ast = assetBuffer("shaders/flatdraw.frag", &tempbuf, &tempbufl);
          check(glShaderSource(fi, 1, (const GLchar **)&tempbuf, (const GLint *)&tempbufl));
          assetClose(ast);
          checkCompileShader(fi);
          check(glAttachShader(src.ui.shader, fi));
          checkLinkProgram(src.ui.shader);
          check(glDeleteShader(vi));
          check(glDeleteShader(fi));
          check(src.ui.uniform_proj = glGetUniformLocation(src.ui.shader, "u_proj"));
          check(src.ui.uniform_tex = glGetUniformLocation(src.ui.shader, "u_tex"));
          check(glGenVertexArrays(1, &src.ui.vao));
          check(glGenBuffers(2, &src.ui.vbo));
          check(glBindVertexArray(src.ui.vao));
          uint16_t indexs[MAX_UI_DRAW * 6];
          for (uint16_t i = 0, j = 0, k = 0; i < MAX_UI_DRAW; i++, j += 6) {
            indexs[j] = k++;
            indexs[j + 1] = indexs[j + 5] = k++;
            indexs[j + 2] = indexs[j + 4] = k++;
            indexs[j + 3] = k++;
          }
          // 0, 1, 2, 3, 2, 1
          check(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, src.ui.ibo));
          check(glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_UI_DRAW * 6 * sizeof(unsigned short), (void *)indexs, GL_STATIC_DRAW));
          check(glBindBuffer(GL_ARRAY_BUFFER, src.ui.vbo));
          check(glBufferData(GL_ARRAY_BUFFER, MAX_UI_DRAW * 4 * sizeof(flat_vertex), NULL, GL_DYNAMIC_DRAW));
          check(glEnableVertexAttribArray(0));
          check(glEnableVertexAttribArray(1));
          check(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(flat_vertex), (void *)0));
          check(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(flat_vertex), (void *)sizeof(vec2)));
        }
        // world draw
        {
          check(src.world.shader = glCreateProgram());
          check(vi = glCreateShader(GL_VERTEX_SHADER));
          ast = assetBuffer("shaders/worlddraw.vert", &tempbuf, &tempbufl);
          check(glShaderSource(vi, 1, (const GLchar **)&tempbuf, (const GLint *)&tempbufl));
          assetClose(ast);
          checkCompileShader(vi);
          check(glAttachShader(src.world.shader, vi));
          check(fi = glCreateShader(GL_FRAGMENT_SHADER));
          ast = assetBuffer("shaders/worlddraw.frag", &tempbuf, &tempbufl);
          check(glShaderSource(fi, 1, (const GLchar **)&tempbuf, (const GLint *)&tempbufl));
          assetClose(ast);
          checkCompileShader(fi);
          check(glAttachShader(src.world.shader, fi));
          checkLinkProgram(src.world.shader);
          check(glDeleteShader(vi));
          check(glDeleteShader(fi));
          check(src.world.uniform_proj = glGetUniformLocation(src.world.shader, "worldview_proj"));
          check(src.world.uniform_transProj = glGetUniformLocation(src.world.shader, "trans_proj"));
        }
      }
      // texture
      // start from 0 to validate default texture
      for (texture t = 0; t < MAX_RESOURCE; ++t) {
        if (src.textures[t].size.x == 0)
          continue;
        check(glGenTextures(1, &src.textures[t].id));
        check(glBindTexture(GL_TEXTURE_2D, src.textures[t].id));
        check(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
        check(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, src.textures[t].size.x, src.textures[t].size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, src.textures[t].data));
        check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        check(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
      }
      check(glBindTexture(GL_TEXTURE_2D, 0));
      // mesh
      for (mesh m = 0; m < MAX_RESOURCE; ++m) {
        if (src.meshes[m].vertex_len == 0)
          continue;
        check(glGenVertexArrays(1, &src.meshes[m].vao));
        check(glGenBuffers(2, &src.meshes[m].vbo));
        check(glBindVertexArray(src.meshes[m].vao));
        check(glBindBuffer(GL_ARRAY_BUFFER, src.meshes[m].vbo));
        check(glBufferData(GL_ARRAY_BUFFER, src.meshes[m].vertex_len * sizeof(mesh_vertex), (void *)src.meshes[m].vertexs, GL_STATIC_DRAW));
        check(glEnableVertexAttribArray(0));
        check(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex), (void *)0));
        check(glEnableVertexAttribArray(1));
        check(glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(mesh_vertex), (void *)sizeof(vec3)));
        check(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, src.meshes[m].ibo));
        check(glBufferData(GL_ELEMENT_ARRAY_BUFFER, src.meshes[m].index_len * sizeof(mesh_index), (void *)src.meshes[m].indices, GL_STATIC_DRAW));
      }
      check(glBindVertexArray(0));
    }
    src.flags |= RESIZE_ONLY;
    src.flags &= ~RESIZE_DISPLAY;
  } else if (src.flags & RESIZE_DISPLAY) {
    eglMakeCurrent(src.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglMakeCurrent(src.display, src.surface, src.surface, src.context);
    src.flags |= RESIZE_ONLY;
    src.flags &= ~RESIZE_DISPLAY;
  }
  if (src.flags & RESIZE_ONLY) {
    EGLint w, h;
    eglQuerySurface(src.display, src.surface, EGL_WIDTH, &w);
    eglQuerySurface(src.display, src.surface, EGL_HEIGHT, &h);
    check(glViewport(0, 0, w, h));
    src.viewportSize.x = (float)w;
    src.viewportSize.y = (float)h;
    src.screenSize.x = src.viewportSize.x - src.insets.x - src.insets.z;
    src.screenSize.y = src.viewportSize.y - src.insets.y - src.insets.w;
    src.flags |= WORLD_UPDATE | UI_UPDATE;
    src.flags &= ~RESIZE_ONLY;
  }
  check(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
  return true;
}
void opengles_postRender(void) {
  if (!eglSwapBuffers(src.display, src.surface)) {
    switch (eglGetError()) {
    case EGL_BAD_SURFACE:
    case EGL_BAD_NATIVE_WINDOW:
    case EGL_BAD_CURRENT_SURFACE:
      killEGL(TERM_EGL_SURFACE);
      break;
    case EGL_BAD_CONTEXT:
    case EGL_CONTEXT_LOST:
      killEGL(TERM_EGL_CONTEXT);
      break;
    case EGL_NOT_INITIALIZED:
    case EGL_BAD_DISPLAY:
      killEGL(TERM_EGL_DISPLAY);
      break;
    default:
      LOGE("EGL error swapbuffers");
      break;
    }
  }
}
void opengles_term(void) {
  killEGL(TERM_EGL_DISPLAY);
  // texture
  for (texture i = 0; i < MAX_RESOURCE; ++i) {
    if (src.textures[i].size.x == 0)
      continue;
    free(src.textures[i].data);
  }
  // mesh
  for (mesh i = 0; i < MAX_RESOURCE; ++i) {
    if (src.meshes[i].vertex_len == 0)
      continue;
    free(src.meshes[i].vertexs);
    free(src.meshes[i].indices);
  }
  if (src.opengles_info_temp) {
    free(src.opengles_info_temp);
  }
  memset(&src, 0, sizeof(src));
}

int opengles_init(void) {
  // add default texture
  {
    src.textures[0].size.x = 1;
    src.textures[0].size.y = 1;
    src.textures[0].data = malloc(4);
    memset(src.textures[0].data, 0xff, 4);
  }
  return 1;
}