#include <android/configuration.h>
#include <android/looper.h>
#include <android/native_activity.h>

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
#include "log.h"

struct msg_pipe {
  int8_t cmd;
  void *data;
};

enum {
  STATE_APP_INITIAL  = 1,
  STATE_APP_WINDOW   = 2,
  STATE_APP_RUNNING  = 4,
  STATE_APP_STARTING = 8,
  STATE_APP_READY    = 15
};

struct android_app {
  ANativeActivity *activity;
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
  static struct msg_pipe rmsg;
  if (read(fd, &rmsg, sizeof(struct msg_pipe)) != sizeof(struct msg_pipe)) {
    LOGE("No data on command pipe!");
    return 0;
  }
  switch (rmsg.cmd) {
    case APP_CMD_SAVE_STATE:
    case APP_CMD_WINDOW_REDRAW:
    case APP_CMD_START:
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
      app->window = CAST(ANativeWindow*)rmsg.data;
      app->stateApp |= STATE_APP_WINDOW;
      break;
    case APP_CMD_RESUME:
      app->stateApp |= STATE_APP_RUNNING;
      break;
    case APP_CMD_WINDOW_RESIZE:
      graphics_resize(true, NULL);
      break;
    case APP_CMD_WINDOW_RESIZE_INSETS:
    case APP_CMD_CONTENT_RECT_CHANGED:
      graphics_resize(false, CAST(float*)rmsg.data);
      free(rmsg.data);
      break;
    case APP_CMD_GAINED_FOCUS:
      // sensor enable
      break;
    case APP_CMD_LOST_FOCUS:
      // sensor disable
      break;
    case APP_CMD_CONFIG_CHANGED:
      AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
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
      // pause
      break;
    case APP_CMD_WINDOW_RESIZE:
    case APP_CMD_CONTENT_RECT_CHANGED:
      // resize
    default:
      // update
      break;
  }
}
static void postProcessCmd() {
  switch (app->delayed_cmdState) {
    case APP_CMD_WINDOW_DESTROYED:
      graphics_invalidate();
      app->stateApp &= ~STATE_APP_WINDOW;
      break;
    case APP_CMD_PAUSE:
      graphics_invalidate();
      app->stateApp &= ~STATE_APP_RUNNING;
      break;
    case APP_CMD_STOP:
      graphics_invalidate();
      app->stateApp &= ~STATE_APP_STARTING;
      break;
    case APP_CMD_DESTROY:
      graphics_invalidate();
      app->stateApp &= ~STATE_APP_INIT;
      break;
  }
}

static void *main_entry(void *UNUSED_ARG(param)) {
  app->config = AConfiguration_new();
  AConfiguration_fromAssetManager(app->config, app->activity->assetManager);

  ALooper *looper = ALooper_prepare(0);
  ALooper_addFd(looper, app->msgread, 1, ALOOPER_EVENT_INPUT, preProcessCmd, NULL);
  
  // initialize
  graphics_initial();

  app->stateApp = STATE_APP_INIT;

  while (app->stateApp) {
    if (ALooper_pollOnce((app->stateApp != STATE_APP_READY) * -1, NULL, NULL, NULL) == ALOOPER_POLL_ERROR)
      LOGE("ALooper_pollOnce returned an error");
    if ((app->stateApp == STATE_APP_READY) && graphics_validate(app->window)) {
      onProcessCmd();
      // TODO: game frame count
      graphics_postRender();
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
  graphics_destroy();

  // Can't touch app object after this.
  return NULL;
}

static struct msg_pipe wmsg;
static void write_cmd(int8_t cmd, void *data) {
  wmsg.cmd = cmd;
  wmsg.data = data;
  while (write(app->msgwrite, &wmsg, sizeof(struct msg_pipe)) != sizeof(struct msg_pipe))
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
static void onStart(ANativeActivity *UNUSED_ARG(activity)) {
  LOGI("onStart activity state");
  write_cmd(APP_CMD_START, NULL);
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
static void onConfigurationChanged(ANativeActivity *UNUSED_ARG(activity)) {
  LOGI("onConfigurationChanged activity state");
  write_cmd(APP_CMD_CONFIG_CHANGED, NULL);
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
  app->activity = activity;

  pthread_mutex_init(&app->mutex, NULL);
  pthread_cond_init(&app->cond, NULL);

  IS_ERROR (pipe(&app->msgread)) {
    LOGE("Failure create pipe onCreate");
    goto oncreate_failure;
  }

  IS_ERROR (pthread_create(&app->thread, NULL, main_entry, NULL)) {
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

#ifdef DEBUG
void toastMessage(const char *msg, ...) {
  if (!app)
    return;

  static char temp[512];
  va_list args;
  va_start(args, msg);
  vsnprintf(temp, 512, msg, args);
  va_end(args);

  JavaVM *vm = app->activity->vm;
  JNIEnv *env;
  static jmethodID id = 0;
  if ((*vm)->AttachCurrentThread(vm, &env, NULL) != JNI_OK) {
    if (!id) {
      jclass cls = (*env)->GetObjectClass(env, app->activity->clazz);
      id = (*env)->GetMethodID(env, cls, "showToast", "(Ljava/lang/String;)V");
    }
    jstring jmsg = (*env)->NewStringUTF(env, temp);
    (*env)->CallVoidMethod(env, app->activity->clazz, id, jmsg);
    (*vm)->DetachCurrentThread(vm);
  }
}
void finish() {
  if (!app)
    return;
  ANativeActivity_finish(app->activity);
}
#endif // DEBUG

// native MainActivity.java
JNIEXPORT void JNICALL Java_com_ariasaproject_gametemplate_MainActivity_insetNative(JNIEnv *UNUSED_ARG(env), jobject UNUSED_ARG(o), jint left, jint top, jint right, jint bottom) {
  if (!app) return;
  float *ins = CAST(float*)malloc(sizeof(float)*4);
  ins[0] = left;
  ins[1] = top;
  ins[2] = right;
  ins[3] = bottom;
  write_cmd(APP_CMD_WINDOW_RESIZE, CAST(void*)ins);
}
