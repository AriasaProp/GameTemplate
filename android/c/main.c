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

#include "common.h"

struct msg_pipe {
  int8_t cmd;
  void *data;
};

enum {
  STATE_APP_DESTROY = 0,
  STATE_APP_INIT    = 1,
  STATE_APP_WINDOW  = 2,
  STATE_APP_RUNNING = 4,
};

struct android_app {
  void *userData;
  ANativeActivity *activity;
  AConfiguration *config;
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
  APP_CMD_WINDOW_REDRAW,
  APP_CMD_CONTENT_RECT_CHANGED,
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

static int process_cmd(int fd, int UNUSED_ARG(event), void *UNUSED_ARG(data)) {
  static struct msg_pipe rmsg;
  if (read(fd, &rmsg, sizeof(struct msg_pipe)) != sizeof(struct msg_pipe)) {
    LOGE("No data on command pipe!");
    return 0;
  }
  switch (rmsg.cmd) {
  case APP_CMD_SAVE_STATE:
  case APP_CMD_WINDOW_REDRAW:
  case APP_CMD_START:
  case APP_CMD_STOP:
    break;
  case APP_CMD_INPUT_CREATED:
    // input init
    break;
  case APP_CMD_INPUT_DESTROYED:
    // input destroy
    break;
  case APP_CMD_WINDOW_CREATED:
    // graphics create
    app->stateApp |= STATE_APP_WINDOW;
    break;
  case APP_CMD_RESUME:
    app->stateApp |= STATE_APP_RUNNING;
    break;
  case APP_CMD_CONTENT_RECT_CHANGED:
    // graphics resize
    break;
  case APP_CMD_WINDOW_RESIZE:
    // graphics resize display
    break;
  case APP_CMD_GAINED_FOCUS:
    // input in sensor enabled
    break;
  case APP_CMD_LOST_FOCUS:
    // input in sensor disabled
    break;
  case APP_CMD_CONFIG_CHANGED:
    AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
    break;
  case APP_CMD_DESTROY:
    app->stateApp &= ~STATE_APP_INIT;
    AConfiguration_delete(app->config);
    break;
  }
  app->delayed_cmdState = rmsg.cmd;
  return 1;
}

static void *android_app_entry(void *UNUSED_ARG(param)) {
  app->config = AConfiguration_new();
  AConfiguration_fromAssetManager(app->config, app->activity->assetManager);

  ALooper *looper = ALooper_prepare(0);
  ALooper_addFd(looper, app->msgread, 1, ALOOPER_EVENT_INPUT, process_cmd, NULL);

  // initialize object
  app->stateApp = STATE_APP_INIT;
  while (app->stateApp) {
    int ready = app->stateApp & (STATE_APP_WINDOW | STATE_APP_RUNNING);
    IS_ERROR (ALooper_pollOnce(!ready * -1, NULL, NULL, NULL) == ALOOPER_POLL_ERROR)
      LOGE("ALooper_pollOnce returned an error");
    if (ready /* graphics pre render */) {
      // update main
      if ((app->delayed_cmdState == APP_CMD_WINDOW_DESTROYED) ||
          (app->delayed_cmdState == APP_CMD_PAUSE)) {
        // pause main
      }
      // counting fps
      // post render
    }
    switch (app->delayed_cmdState) {
    case APP_CMD_WINDOW_DESTROYED:
      // graphics destroy
      app->stateApp &= ~STATE_APP_WINDOW;
      break;
    case APP_CMD_PAUSE:
      app->stateApp &= ~STATE_APP_RUNNING;
    }
    if (app->cmdState != app->delayed_cmdState) {
      pthread_mutex_lock(&app->mutex);
      app->cmdState = app->delayed_cmdState;
      pthread_cond_signal(&app->cond);
      pthread_mutex_unlock(&app->mutex);
    }
  }
  // Don't touch app object after this.
  return NULL;
}

static struct msg_pipe wmsg;
static inline void write_cmd(int8_t cmd, void *data) {
  wmsg.cmd = cmd;
  wmsg.data = data;
  IS_ERROR (write(app->msgwrite, &wmsg, sizeof(struct msg_pipe)) != sizeof(struct msg_pipe)) {
    LOGE("Failure writing android_app cmd: %s\n", strerror(errno));
    write_cmd(cmd, data);
  }
}
static inline void write_cmd_and_wait(int8_t cmd, void *data) {
  write_cmd(cmd, data);
  pthread_mutex_lock(&app->mutex);
  while (app->cmdState != cmd)
    pthread_cond_wait(&app->cond, &app->mutex);
  pthread_mutex_unlock(&app->mutex);
}

static void onDestroy(ANativeActivity *UNUSED_ARG(activity)) {
  write_cmd(APP_CMD_DESTROY, NULL);
  pthread_mutex_lock(&app->mutex);
  while (app->stateApp)
    pthread_cond_wait(&app->cond, &app->mutex);
  pthread_mutex_unlock(&app->mutex);

  close(app->msgread);
  close(app->msgwrite);
  pthread_cond_destroy(&app->cond);
  pthread_mutex_destroy(&app->mutex);
  free(app);
  app = NULL;
}
static void onStart(ANativeActivity *UNUSED_ARG(activity)) {
  write_cmd(APP_CMD_START, NULL);
}
static void onResume(ANativeActivity *UNUSED_ARG(activity)) {
  write_cmd(APP_CMD_RESUME, NULL);
}
static void *onSaveInstanceState(ANativeActivity *UNUSED_ARG(activity), size_t *UNUSED_ARG(outLen)) {
  // TODO: save data state 
  write_cmd(APP_CMD_SAVE_STATE, NULL);
  return NULL;
}
static void onPause(ANativeActivity *UNUSED_ARG(activity)) {
  write_cmd_and_wait(APP_CMD_PAUSE, NULL);
}
static void onStop(ANativeActivity *UNUSED_ARG(activity)) {
  write_cmd_and_wait(APP_CMD_STOP, NULL);
}
static void onConfigurationChanged(ANativeActivity *UNUSED_ARG(activity)) {
  write_cmd(APP_CMD_CONFIG_CHANGED, NULL);
}
static void onLowMemory(ANativeActivity *UNUSED_ARG(activity)) {
  write_cmd_and_wait(APP_CMD_LOW_MEMORY, NULL);
}
static void onWindowFocusChanged(ANativeActivity *UNUSED_ARG(activity), int focused) {
  write_cmd(focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS, NULL);
}
static void onContentRectChanged(ANativeActivity *UNUSED_ARG(activity), const ARect *rect) {
  write_cmd(APP_CMD_CONTENT_RECT_CHANGED, (void *)rect);
}
static void onNativeWindowResized(ANativeActivity *UNUSED_ARG(activity), ANativeWindow *UNUSED_ARG(window)) {
  write_cmd(APP_CMD_WINDOW_RESIZE, NULL);
}
static void onNativeWindowRedrawNeeded(ANativeActivity *UNUSED_ARG(activity), ANativeWindow *UNUSED_ARG(window)) {
  write_cmd(APP_CMD_WINDOW_REDRAW, NULL);
}
static void onNativeWindowCreated(ANativeActivity *UNUSED_ARG(activity), ANativeWindow *window) {
  write_cmd(APP_CMD_WINDOW_CREATED, (void *)window);
}
static void onNativeWindowDestroyed(ANativeActivity *UNUSED_ARG(activity), ANativeWindow *UNUSED_ARG(window)) {
  write_cmd_and_wait(APP_CMD_WINDOW_DESTROYED, NULL);
}
static void onInputQueueCreated(ANativeActivity *UNUSED_ARG(activity), AInputQueue *queue) {
  write_cmd(APP_CMD_INPUT_CREATED, (void *)queue);
}
static void onInputQueueDestroyed(ANativeActivity *UNUSED_ARG(activity), AInputQueue *UNUSED_ARG(queue)) {
  write_cmd_and_wait(APP_CMD_INPUT_DESTROYED, NULL);
}
void ANativeActivity_onCreate(ANativeActivity *activity, void *savedState, size_t savedStateSize) {
#define SET_CALLBACK(A) activity->callbacks->A = A
  SET_CALLBACK(onDestroy);
  SET_CALLBACK(onStart);
  SET_CALLBACK(onResume);
  SET_CALLBACK(onSaveInstanceState);
  SET_CALLBACK(onPause);
  SET_CALLBACK(onStop);
  SET_CALLBACK(onConfigurationChanged);
  SET_CALLBACK(onLowMemory);
  SET_CALLBACK(onWindowFocusChanged);
  SET_CALLBACK(onContentRectChanged);
  SET_CALLBACK(onNativeWindowResized);
  SET_CALLBACK(onNativeWindowRedrawNeeded);
  SET_CALLBACK(onNativeWindowCreated);
  SET_CALLBACK(onNativeWindowDestroyed);
  SET_CALLBACK(onInputQueueCreated);
  SET_CALLBACK(onInputQueueDestroyed);
#undef SET_CALLBACK
  app = (struct android_app *)calloc(1, sizeof(struct android_app));
  app->activity = activity;

  pthread_mutex_init(&app->mutex, NULL);
  pthread_cond_init(&app->cond, NULL);
  // TODO: state loaded for recreate
  IS_ERROR (pipe(&app->msgread))
    goto oncreate_failure;

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  IS_ERROR (pthread_create(&app->thread, &attr, android_app_entry, NULL)) {
    pthread_attr_destroy(&attr);
    goto oncreate_failure;
  }
  pthread_attr_destroy(&attr);
  return;
oncreate_failure:
  // unset callback
  memset(activity->callbacks, 0, sizeof(*activity->callbacks));
  // freed android resources
  free(app);
  app = NULL;
  ANativeActivity_finish(activity);
}

#ifdef _DEBUG
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
#endif // _DEBUG

// native MainActivity.java
JNIEXPORT void JNICALL Java_com_ariasaproject_technowar_MainActivity_insetNative(JNIEnv *UNUSED_ARG(env), jobject UNUSED_ARG(o), jint UNUSED_ARG(left), jint UNUSED_ARG(top), jint UNUSED_ARG(right), jint UNUSED_ARG(bottom)) {
  // TODO: onGraphics resize on insets
}
