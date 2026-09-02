#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>
#include <android/native_window.h>
#include <android/asset_manager.h>

#include <errno.h>
#include <jni.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

#include "graphics/graphics.h"
#include "common.h"
#include "main.h"
#include "log.h"
// assets
extern void androidAsset_init(AAssetManager*);
extern void androidAsset_term(void);
// graphics
extern void androidGraphics_initial(void);
extern void androidGraphics_attach(ANativeWindow*);
extern bool androidGraphics_validate(void);
extern void androidGraphics_resize(bool,float*);
extern void androidGraphics_postRender(void);
extern void androidGraphics_invalidate(void);
extern void androidGraphics_destroy(void);


typedef struct {
  int8_t cmd;
  void *data;
} msg_pipe;

enum {
  STATE_APP_INITIAL  = 1 << 0,
  STATE_APP_WINDOW   = 1 << 1,
  STATE_APP_RUNNING  = 1 << 2,
  STATE_APP_STARTING = 1 << 3,
  STATE_APP_READY    = 15
};

struct android_app {
  AConfiguration *config;
  ANativeWindow *window;
  ARect contentRect;

  int8_t delayed_cmdState;
  int8_t cmdState;

  pthread_mutex_t mutex;
  pthread_cond_t cond;

  int msgread, msgwrite;

  pthread_t thread;

  int stateApp;
} *app = NULL;

enum {
  APP_CMD_NONE,
  APP_CMD_INPUT_CREATED,
  APP_CMD_INPUT_DESTROYED,
  APP_CMD_WINDOW_CREATED,
  APP_CMD_WINDOW_DESTROYED,
  APP_CMD_WINDOW_RESIZE,
  APP_CMD_CONTENT_RECT_CHANGED,
  APP_CMD_WINDOW_RESIZE_INSETS,
  APP_CMD_WINDOW_REDRAW,
  APP_CMD_GAINED_FOCUS,
  APP_CMD_LOST_FOCUS,
  APP_CMD_CONFIG_CHANGED,
  APP_CMD_LOW_MEMORY,
  APP_CMD_START,
  APP_CMD_RESUME,
  APP_CMD_SAVE_STATE,
  APP_CMD_PAUSE,
  APP_CMD_STOP,
  APP_CMD_DESTROY,
};

static int preProcessCmd(int fd, int UNUSED_ARG(event), void *UNUSED_ARG(data)) {
  static msg_pipe rmsg;
  if (read(fd, &rmsg, sizeof(msg_pipe)) != sizeof(msg_pipe)) {
    LOGE("No data on command pipe!");
    return 0;
  }
  switch (rmsg.cmd) {
    case APP_CMD_SAVE_STATE:
    case APP_CMD_WINDOW_REDRAW:
      break;
    case APP_CMD_START:
      androidAsset_init(CAST(AAssetManager*)rmsg.data);
      app->stateApp |= STATE_APP_STARTING;
      break;
    case APP_CMD_INPUT_CREATED:
      // input create
      break;
    case APP_CMD_INPUT_DESTROYED:
      // input destroy
      break;
    case APP_CMD_WINDOW_CREATED:
      // graphics create
      androidGraphics_attach(CAST(ANativeWindow*)rmsg.data);
      app->stateApp |= STATE_APP_WINDOW;
      break;
    case APP_CMD_RESUME:
      app->stateApp |= STATE_APP_RUNNING;
      break;
    case APP_CMD_WINDOW_RESIZE:
      androidGraphics_resize(true, VEC4_ZERO);
      break;
    case APP_CMD_WINDOW_RESIZE_INSETS:
    case APP_CMD_CONTENT_RECT_CHANGED:
      androidGraphics_resize(false, *CAST(vec4*)rmsg.data);
      free(rmsg.data);
      break;
    case APP_CMD_GAINED_FOCUS:
      // sensor enable
      break;
    case APP_CMD_LOST_FOCUS:
      // sensor disable
      break;
    case APP_CMD_CONFIG_CHANGED:
      AConfiguration_fromAssetManager(app->config, CAST(AAssetManager*)rmsg.data);
      break;
  }
  app->delayed_cmdState = rmsg.cmd;
  return 1;
}
static void onProcessCmd() {
  switch (app->delayed_cmdState) {
    case APP_CMD_WINDOW_DESTROYED:
    case APP_CMD_PAUSE:
    case APP_CMD_STOP:
    case APP_CMD_DESTROY:
      main_pause();
      break;
    case APP_CMD_WINDOW_RESIZE:
    case APP_CMD_CONTENT_RECT_CHANGED:
      // resize
    default:
      main_update();
      break;
  }
}
static void postProcessCmd() {
  switch (app->delayed_cmdState) {
    case APP_CMD_DESTROY:
      app->stateApp &= ~STATE_APP_INITIAL;
    case APP_CMD_STOP:
      app->stateApp &= ~STATE_APP_STARTING;
      androidAsset_term();
    case APP_CMD_PAUSE:
      app->stateApp &= ~STATE_APP_RUNNING;
    case APP_CMD_WINDOW_DESTROYED:
      androidGraphics_invalidate();
      app->stateApp &= ~STATE_APP_WINDOW;
      break;
  }
}

static void *main_entry(void *param) {
  app->config = AConfiguration_new();
  AConfiguration_fromAssetManager(app->config, CAST(AAssetManager*)param);

  ALooper *looper = ALooper_prepare(0);
  ALooper_addFd(looper, app->msgread, 1, ALOOPER_EVENT_INPUT, preProcessCmd, NULL);
  
  // initialize
  androidGraphics_initial();

  app->stateApp = STATE_APP_INITIAL;

  while (app->stateApp) {
    if (ALooper_pollOnce((app->stateApp != STATE_APP_READY) * -1, NULL, NULL, NULL) == ALOOPER_POLL_ERROR)
      LOGE("ALooper_pollOnce returned an error");
    if ((app->stateApp == STATE_APP_READY) && androidGraphics_validate()) {
      onProcessCmd();
      // TODO: game frame count
      androidGraphics_postRender();
    }
    postProcessCmd();
    if (app->cmdState != app->delayed_cmdState) {
      pthread_mutex_lock(&app->mutex);
      app->cmdState = app->delayed_cmdState;
      pthread_cond_signal(&app->cond);
      pthread_mutex_unlock(&app->mutex);
    }
  }
  AConfiguration_delete(app->config);
  androidGraphics_destroy();

  // Can't touch app object after this.
  return NULL;
}

static msg_pipe wmsg;
static void write_cmd(int8_t cmd, void *data) {
  wmsg.cmd = cmd;
  wmsg.data = data;
  while (write(app->msgwrite, &wmsg, sizeof(msg_pipe)) != sizeof(msg_pipe))
    LOGE("Failure writing android_app cmd: %s\n", strerror(errno));
}
static inline void write_cmd_and_wait(int8_t cmd, void *data) {
  write_cmd(cmd, data);
  pthread_mutex_lock(&app->mutex);
  while (app->cmdState != cmd)
    pthread_cond_wait(&app->cond, &app->mutex);
  pthread_mutex_unlock(&app->mutex);
}

static void onDestroy(ANativeActivity *UNUSED_ARG(activity)) {
  LOGI("onDestroy activity state");
  write_cmd(APP_CMD_DESTROY, NULL);
  pthread_join(app->thread, NULL);

  close(app->msgread);
  close(app->msgwrite);
  pthread_cond_destroy(&app->cond);
  pthread_mutex_destroy(&app->mutex);
  free(app);
  app = NULL;
  LOGI("onDestroy activity state done");
}
static void onStart(ANativeActivity *activity) {
  LOGI("onStart activity state");
  write_cmd(APP_CMD_START, CAST(void*)activity->assetManager);
}
static void onResume(ANativeActivity *UNUSED_ARG(activity)) {
  LOGI("onResume activity state");
  write_cmd(APP_CMD_RESUME, NULL);
}
static void *onSaveInstanceState(ANativeActivity *UNUSED_ARG(activity), size_t *UNUSED_ARG(outLen)) {
  LOGI("onSaveInstanceState activity state");
  write_cmd(APP_CMD_SAVE_STATE, NULL);
  return NULL;
}
static void onPause(ANativeActivity *UNUSED_ARG(activity)) {
  LOGI("onPause activity state");
  write_cmd_and_wait(APP_CMD_PAUSE, NULL);
}
static void onStop(ANativeActivity *UNUSED_ARG(activity)) {
  LOGI("onStop activity state");
  write_cmd_and_wait(APP_CMD_STOP, NULL);
}
static void onConfigurationChanged(ANativeActivity *activity) {
  LOGI("onConfigurationChanged activity state");
  write_cmd(APP_CMD_CONFIG_CHANGED, CAST(void*)activity->assetManager);
}
static void onLowMemory(ANativeActivity *UNUSED_ARG(activity)) {
  LOGI("onLowMemory activity state");
  write_cmd(APP_CMD_LOW_MEMORY, NULL);
}
static void onWindowFocusChanged(ANativeActivity *UNUSED_ARG(activity), int focused) {
  LOGI("onWindowFocusChanged activity state");
  write_cmd(focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS, NULL);
}
static void onContentRectChanged(ANativeActivity *UNUSED_ARG(activity), const ARect *rect) {
  LOGI("onContentRectChanged activity state");
  write_cmd(APP_CMD_CONTENT_RECT_CHANGED, (void *)rect);
}
static void onNativeWindowResized(ANativeActivity *UNUSED_ARG(activity), ANativeWindow *UNUSED_ARG(window)) {
  LOGI("onNativeWindowResized activity state");
  write_cmd(APP_CMD_WINDOW_RESIZE, NULL);
}
static void onNativeWindowRedrawNeeded(ANativeActivity *UNUSED_ARG(activity), ANativeWindow *UNUSED_ARG(window)) {
  LOGI("onNativeWindowRedrawNeeded activity state");
  write_cmd(APP_CMD_WINDOW_REDRAW, NULL);
}
static void onNativeWindowCreated(ANativeActivity *UNUSED_ARG(activity), ANativeWindow *window) {
  LOGI("onNativeWindowCreated activity state");
  write_cmd_and_wait(APP_CMD_WINDOW_CREATED, (void *)window);
}
static void onNativeWindowDestroyed(ANativeActivity *UNUSED_ARG(activity), ANativeWindow *UNUSED_ARG(window)) {
  LOGI("onNativeWindowDestroyed activity state");
  write_cmd_and_wait(APP_CMD_WINDOW_DESTROYED, NULL);
}
static void onInputQueueCreated(ANativeActivity *UNUSED_ARG(activity), AInputQueue *queue) {
  LOGI("onInputQueueCreated activity state");
  write_cmd(APP_CMD_INPUT_CREATED, (void *)queue);
}
static void onInputQueueDestroyed(ANativeActivity *UNUSED_ARG(activity), AInputQueue *UNUSED_ARG(queue)) {
  LOGI("onInputQueueDestroyed activity state");
  write_cmd(APP_CMD_INPUT_DESTROYED, NULL);
}
void ANativeActivity_onCreate(ANativeActivity *activity, void *UNUSED_ARG(savedState), size_t UNUSED_ARG(savedStateSize)) {
  LOGI("onCreate activity state");
#define CALLBACK_SET(A) activity->callbacks->A = A
  CALLBACK_SET(onDestroy);
  CALLBACK_SET(onStart);
  CALLBACK_SET(onResume);
  CALLBACK_SET(onSaveInstanceState);
  CALLBACK_SET(onPause);
  CALLBACK_SET(onStop);
  CALLBACK_SET(onConfigurationChanged);
  CALLBACK_SET(onLowMemory);
  CALLBACK_SET(onWindowFocusChanged);
  CALLBACK_SET(onContentRectChanged);
  CALLBACK_SET(onNativeWindowResized);
  CALLBACK_SET(onNativeWindowRedrawNeeded);
  CALLBACK_SET(onNativeWindowCreated);
  CALLBACK_SET(onNativeWindowDestroyed);
  CALLBACK_SET(onInputQueueCreated);
  CALLBACK_SET(onInputQueueDestroyed);
#undef CALLBACK_SET
  
  app = (struct android_app *)calloc(1, sizeof(struct android_app));

  pthread_mutex_init(&app->mutex, NULL);
  pthread_cond_init(&app->cond, NULL);

  IS_ERROR (pipe(&app->msgread)) {
    LOGE("Failure create pipe onCreate");
    goto oncreate_failure;
  }

  IS_ERROR (pthread_create(&app->thread, NULL, main_entry, CAST(void*)activity->assetManager)) {
    LOGE("Failure running thread");
    close(app->msgread);
    close(app->msgwrite);
    goto oncreate_failure;
  }
  LOGI("onCreate activity state done");
  return;
oncreate_failure:
  pthread_mutex_destroy(&app->mutex);
  pthread_cond_destroy(&app->cond);
  ANativeActivity_finish(activity);
  free(app);
  app = NULL;
  LOGI("onCreate activity state failure");
}

// native MainActivity.java
JNIEXPORT void JNICALL Java_com_ariasaproject_gametemplate_MainActivity_insetNative(JNIEnv *UNUSED_ARG(env), jobject UNUSED_ARG(o), jint left, jint top, jint right, jint bottom) {
  if (!app) return;
  vec4 *ins = CAST(float*)malloc(sizeof(vec4));
  ins->x = left;
  ins->y = top;
  ins->z = right;
  ins->w = bottom;
  write_cmd(APP_CMD_WINDOW_RESIZE, CAST(void*)ins);
}
