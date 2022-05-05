#ifdef __APPLE__ // darwin
#include <TargetConditionals.h>
#endif // __APPLE__

#if defined(__i386__) || defined(_M_IX86) // x86
#if defined(__ANDROID__) // x86-android
#include "curl_config.x86-android.h"
#elif TARGET_OS_IPHONE // x86-ios
#include "curl_config.x86-ios.h"
#elif TARGET_OS_OSX
#include "curl_config.x86-mac.h"
#endif

#elif defined(__x86_64__) || defined(_M_X64) // x86_64
#if defined(__ANDROID__) // x86_64-android
#include "curl_config.x86_64-android.h"
#elif TARGET_OS_IPHONE // x86_64-ios
#include "curl_config.x86_64-ios.h"
#elif TARGET_OS_OSX
#include "curl_config.x86_64-mac.h"
#else
#include "curl_config.x86_64.h"
#endif

#elif defined(__arm__) || defined(_M_ARM) // arm
#if defined(__ANDROID__) // arm-android
#include "curl_config.arm-android.h"
#elif TARGET_OS_IPHONE // arm-ios
#include "curl_config.arm-ios.h"
#endif

#elif defined(__aarch64__) // arm64
#if defined(__ANDROID__) // arm64-android
#include "curl_config.arm64-android.h"
#elif TARGET_OS_IPHONE // arm64-ios
#include "curl_config.arm64-ios.h"
#elif TARGET_OS_IPHONE // arm64-mac
#include "curl_config.arm64-mac.h"
#endif

#endif
