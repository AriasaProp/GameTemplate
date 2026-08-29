#include "common.h"
#include "math/color.h"
#include "graphics/graphics.h"
#include "log.h"

#include <android/native_window.h>
#include <dlfcn.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <GLES3/gl32.h>
#include <EGL/egl.h>

#define MAX_ASSET_READING 256
#define MAX_UI_DRAW       200
#define MAX_MSG           512
#define check(X) do { \
  X;\
  for (GLenum error; (error = glGetError()); ) \
    LOGE("Err %s 0x%x\n", X, error); \
} while (0)

static GLint success;
static GLchar msg[MAX_MSG];

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
static struct {
  ANativeWindow *window;
  EGLDisplay display;
  EGLSurface surface;
  EGLContext context;
  EGLConfig eConfig;
  int flags;
  bool shader_valid;
  
  struct {
    GLint shader, uniform_proj, uniform_tex;
    GLuint vao, vbo, ibo;
  } ui;
  struct {
    GLint shader, uniform_proj, uniform_transProj;
  } world;
} src = {0};

static void killEGL(const int EGLTermReq) {
  if (!EGLTermReq || !src.display)
    return;
  if (src.shader_valid) {
    // world
    check(glDeleteProgram(src.world.shader));
    // flat draw
    check(glDeleteProgram(src.ui.shader));
    check(glDeleteVertexArrays(1, &src.ui.vao));
    check(glDeleteBuffers(2, &src.ui.vbo));
    // 
    src.shader_valid = false;
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
static void opengles_resize(bool s, uint *data) {
  src.flags |= s ? RESIZE_DISPLAY : RESIZE_ONLY;
  if (data) {
    src.insets.x = data[0];
    src.insets.y = data[1];
    src.insets.z = data[2];
    src.insets.w = data[3];
    src.flags |= UI_UPDATE;
  }
}
bool opengles_validate(ANativeWindow *window) {
  if (!src.window || !src.display || !src.context || !src.surface) {
    if (!src.window) src.window = window;
    if (!src.display) {
      src.context = EGL_NO_CONTEXT;
      src.surface = EGL_NO_SURFACE;
      src.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
      if (!src.display) {
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
    if (!src.context && !(src.context = eglCreateContext(src.display, src.eConfig, NULL, CLIT(EGLint[]){EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE}))) {
      LOGW("Failed to create EGLContext");
      return false;
    }
    if (!src.surface && !(src.surface = eglCreateWindowSurface(src.display, src.eConfig, src.window, NULL))) {
      LOGW("Failed to create EGLSurface");
      return false;
    }
    eglMakeCurrent(src.display, src.surface, src.surface, src.context);
    if (!src.shader_valid) {
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
        void *ast;
        size_t tempbufl;
        GLuint vi, fi;
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
        // flat draw
        {
          check(src.ui.shader = glCreateProgram());
          check(vi = glCreateShader(GL_VERTEX_SHADER));
          ast = global_engine.assetBuffer("shaders/flatdraw.vert", &tempbuf, &tempbufl);
          check(glShaderSource(vi, 1, (const GLchar **)&tempbuf, (const GLint *)&tempbufl));
          global_engine.assetClose(ast);
          checkCompileShader(vi);
          check(glAttachShader(src.ui.shader, vi));
          check(fi = glCreateShader(GL_FRAGMENT_SHADER));
          ast = global_engine.assetBuffer("shaders/flatdraw.frag", &tempbuf, &tempbufl);
          check(glShaderSource(fi, 1, (const GLchar **)&tempbuf, (const GLint *)&tempbufl));
          global_engine.assetClose(ast);
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
          ast = global_engine.assetBuffer("shaders/worlddraw.vert", &tempbuf, &tempbufl);
          check(glShaderSource(vi, 1, (const GLchar **)&tempbuf, (const GLint *)&tempbufl));
          global_engine.assetClose(ast);
          checkCompileShader(vi);
          check(glAttachShader(src.world.shader, vi));
          check(fi = glCreateShader(GL_FRAGMENT_SHADER));
          ast = global_engine.assetBuffer("shaders/worlddraw.frag", &tempbuf, &tempbufl);
          check(glShaderSource(fi, 1, (const GLchar **)&tempbuf, (const GLint *)&tempbufl));
          global_engine.assetClose(ast);
          checkCompileShader(fi);
          check(glAttachShader(src.world.shader, fi));
          checkLinkProgram(src.world.shader);
          check(glDeleteShader(vi));
          check(glDeleteShader(fi));
          check(src.world.uniform_proj = glGetUniformLocation(src.world.shader, "worldview_proj"));
          check(src.world.uniform_transProj = glGetUniformLocation(src.world.shader, "trans_proj"));
        }
#undef checkLinkProgram
#undef checkCompileShader
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
      
      src.shader_valid = true;
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
  if (src.flags & (WORLD_UPDATE|UI_UPDATE)) {
    mat4 m = MAT4_IDT;
    m.m[0][0] = 2.f / src.viewportSize.x;
    m.m[1][1] = 2.f / src.viewportSize.y;
    if (src.flags & WORLD_UPDATE) {
      check(glUseProgram(src.world.shader));
      check(glUniformMatrix4fv(src.world.uniform_proj, 1, GL_FALSE, m.v));
      src.flags &= ~WORLD_UPDATE;
    }
    check(glUseProgram(0));
  }
  check(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
  return 1;
}
void opengles_postRender(void) {
  // ui render
  if (src.flags & UI_UPDATE) {
    check(glUseProgram(src.ui.shader));
    m.m[3][0] = (2.f * src.insets.x / src.viewportSize.x) - 1.f;
    m.m[3][1] = (2.f * src.insets.w / src.viewportSize.y) - 1.f;
    check(glUniformMatrix4fv(src->ui.uniform_proj, 1, GL_FALSE, m.v));
    src->flags &= ~UI_UPDATE;
  }
  check(glUseProgram(0));
  
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
void opengles_invalidate(void) {
  killEGL(TERM_EGL_SURFACE);
  src.window = NULL;
}
void opengles_destroy(void) {
  killEGL(TERM_EGL_DISPLAY);
  memset(&src, 0, sizeof(src));
}