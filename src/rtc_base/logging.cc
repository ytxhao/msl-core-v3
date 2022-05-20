/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "rtc_base/logging.h"
#include "rtc_base/time_utils.h"

#include <string.h>

#ifdef _WIN32

#include <io.h>      // _get_osfhandle and _isatty support
#include <process.h> //  _get_pid support
#include <spdlog/details/windows_include.h>

#ifdef __MINGW32__
#include <share.h>
#endif

#if defined(SPDLOG_WCHAR_TO_UTF8_SUPPORT) || defined(SPDLOG_WCHAR_FILENAMES)
#include <limits>
#endif

#include <direct.h> // for _mkdir/_wmkdir

#else // unix

#include <fcntl.h>
#include <unistd.h>

#ifdef __linux__
#include <sys/syscall.h> //Use gettid() syscall under linux to get thread id

#elif defined(_AIX)
#include <pthread.h> // for pthread_getthreadid_np

#elif defined(__DragonFly__) || defined(__FreeBSD__)
#include <pthread_np.h> // for pthread_getthreadid_np

#elif defined(__NetBSD__)
#include <lwp.h> // for _lwp_self

#elif defined(__sun)
#include <thread.h> // for thr_self
#endif

#endif // unix

#if RTC_LOG_ENABLED()

#if defined(WEBRTC_WIN)
#include <windows.h>
#if _MSC_VER < 1900
#define snprintf _snprintf
#endif
#undef ERROR  // wingdi.h
#endif

#if defined(WEBRTC_MAC) && !defined(WEBRTC_IOS)
#include <CoreServices/CoreServices.h>
#elif defined(WEBRTC_ANDROID)
#include <android/log.h>

// Android has a 1024 limit on log inputs. We use 60 chars as an
// approx for the header/tag portion.
// See android/system/core/liblog/logd_write.c
static const int kMaxLogLineSize = 1024 - 60;
#endif  // WEBRTC_MAC && !defined(WEBRTC_IOS) || WEBRTC_ANDROID

#include <inttypes.h>
#include <stdio.h>
#include <time.h>

#include <algorithm>
#include <cstdarg>
#include <vector>

#include "rtc_base/time_utils.h"


namespace rtc {

bool log_to_stderr_ = true;

void SetLogToStderr(bool log_to_stderr) {
  log_to_stderr_ = log_to_stderr;
}

int pid()
{

#ifdef _WIN32
    return static_cast<int>(::GetCurrentProcessId());
#else
    return static_cast<int>(::getpid());
#endif
}

size_t thread_id()
{
#ifdef _WIN32
    return static_cast<size_t>(::GetCurrentThreadId());
#elif defined(__linux__)
#if defined(__ANDROID__) && defined(__ANDROID_API__) && (__ANDROID_API__ < 21)
#define SYS_gettid __NR_gettid
#endif
    return static_cast<size_t>(::syscall(SYS_gettid));
#elif defined(_AIX) || defined(__DragonFly__) || defined(__FreeBSD__)
    return static_cast<size_t>(::pthread_getthreadid_np());
#elif defined(__NetBSD__)
    return static_cast<size_t>(::_lwp_self());
#elif defined(__OpenBSD__)
    return static_cast<size_t>(::getthrid());
#elif defined(__sun)
    return static_cast<size_t>(::thr_self());
#elif __APPLE__
    uint64_t tid;
    pthread_threadid_np(nullptr, &tid);
    return static_cast<size_t>(tid);
#else // Default to standard C++11 (other Unix)
    return static_cast<size_t>(std::hash<std::thread::id>()(std::this_thread::get_id()));
#endif
}

// Return the filename portion of the string (that following the last slash).
const char* FilenameFromPath(const char* file) {
  const char* end1 = ::strrchr(file, '/');
  const char* end2 = ::strrchr(file, '\\');
  if (!end1 && !end2)
    return file;
  else
    return (end1 > end2) ? end1 + 1 : end2 + 1;
}

void MslPrint(LoggingSeverity sev, const char* file, int line_num, const char* tag, const char* fmt, ...) {
  bool log_to_stderr = log_to_stderr_;
  char now[64];
  // int64_t start_time_ms = rtc::TimeUTCMillis();
  int64_t currentMicros = rtc::TimeUTCMicros();
  time_t currentMillis = currentMicros / kNumMicrosecsPerMillisec;
  time_t currentSeconds = currentMillis / 1000;
  struct std::tm *ttime;
  char buf[2048] = {'\0'};
  va_list args;
  va_start(args, fmt); 
  vsnprintf(buf, 2048, fmt, args);
  va_end(args);
  ttime = localtime(&currentSeconds);
  std::strftime(now, 64, "%Y-%m-%d %H:%M:%S", ttime);
  std::stringstream ss;
  const char *file_name_ptr = strrchr(file, '/');
  std::string file_name(file_name_ptr ? file_name_ptr + 1 : file);

  std::string prio;
  switch (sev) {
    case LS_VERBOSE:
      prio = "V/";
      break;
    case LS_ZORRO:
    case LS_INFO:
      prio = "I/";
      break;
    case LS_WARNING:
      prio = "W/";
      break;
    case LS_ERROR:
      prio = "E/";
      break;
    default:
      prio = "V/";
  }
  ss << now << "." << std::to_string(currentMicros % 1000000) 
     << " " + std::to_string(pid()) << "-" << std::to_string(thread_id())
     << " [" << file_name << ":" << line_num << "] " << prio << (tag == nullptr ? "msl" : tag)<< ": "<< std::string(buf) << std::endl;
  std::string str(ss.str());
  // std::string str("");
#if defined(WEBRTC_MAC) && !defined(WEBRTC_IOS) && defined(NDEBUG)
  // On the Mac, all stderr output goes to the Console log and causes clutter.
  // So in opt builds, don't log to stderr unless the user specifically sets
  // a preference to do so.
  CFStringRef key = CFStringCreateWithCString(
      kCFAllocatorDefault, "logToStdErr", kCFStringEncodingUTF8);
  CFStringRef domain = CFBundleGetIdentifier(CFBundleGetMainBundle());
  if (key != nullptr && domain != nullptr) {
    Boolean exists_and_is_valid;
    Boolean should_log =
        CFPreferencesGetAppBooleanValue(key, domain, &exists_and_is_valid);
    // If the key doesn't exist or is invalid or is false, we will not log to
    // stderr.
    log_to_stderr = exists_and_is_valid && should_log;
  }
  if (key != nullptr) {
    CFRelease(key);
  }
#endif  // defined(WEBRTC_MAC) && !defined(WEBRTC_IOS) && defined(NDEBUG)

#if defined(WEBRTC_ANDROID)
  // Android's logging facility uses severity to log messages but we
  // need to map libjingle's severity levels to Android ones first.
  // Also write to stderr which maybe available to executable started
  // from the shell.
  SetLogToStderr(false);
  int prio;
  switch (sev) {
    case LS_VERBOSE:
      prio = ANDROID_LOG_VERBOSE;
      break;
    case LS_ZORRO:
    case LS_INFO:
      prio = ANDROID_LOG_INFO;
      break;
    case LS_WARNING:
      prio = ANDROID_LOG_WARN;
      break;
    case LS_ERROR:
      prio = ANDROID_LOG_ERROR;
      break;
    default:
      prio = ANDROID_LOG_UNKNOWN;
  }

  int size = str.size();
  int line = 0;
  int idx = 0;
  const int max_lines = size / kMaxLogLineSize + 1;
  if (max_lines == 1) {
    __android_log_print(prio, tag, "%.*s", size, str.c_str());
  } else {
    while (size > 0) {
      const int len = std::min(size, kMaxLogLineSize);
      // Use the size of the string in the format (str may have \0 in the
      // middle).
      __android_log_print(prio, tag, "[%d/%d] %.*s", line + 1, max_lines, len,
                          str.c_str() + idx);
      idx += len;
      size -= len;
      ++line;
    }
  }
#endif  // WEBRTC_ANDROID
  if (log_to_stderr) {
    fprintf(stderr, "%s", str.c_str());
    fflush(stderr);
  }
}

}  // namespace rtc

#endif //RTC_LOG_ENABLED
