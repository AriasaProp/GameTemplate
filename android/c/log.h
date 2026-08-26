#ifndef __LOG__
#define __LOG__
#include <android/log.h>

#ifndef _IDENTITY_
#define _IDENTITY_ No_Defined
#endif // _IDENTITY_

#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,#_IDENTITY_,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN ,#_IDENTITY_,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,#_IDENTITY_,__VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO ,#_IDENTITY_,__VA_ARGS__)

#endif // __LOG__