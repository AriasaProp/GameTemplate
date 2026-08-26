#ifndef __LOG__
#define __LOG__

#include <android/log.h>

#ifdef DEBUG
#  define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, _IDENTITY_, __VA_ARGS__))
#endif // DEBUG

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO , _IDENTITY_, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN , _IDENTITY_, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, _IDENTITY_, __VA_ARGS__))

#endif // __LOG__