
#ifndef RTC_BASE_LOGGING_H_
#define RTC_BASE_LOGGING_H_

#include <errno.h>

#include <atomic>
#include <sstream>  // no-presubmit-check TODO(webrtc:8982)
#include <string>
#include <utility>

#if !defined(NDEBUG) || defined(DLOG_ALWAYS_ON)
#define RTC_DLOG_IS_ON 1
#else
#define RTC_DLOG_IS_ON 0
#endif

#if defined(RTC_DISABLE_LOGGING)
#define RTC_LOG_ENABLED() 0
#else
#define RTC_LOG_ENABLED() 1
#endif

namespace rtc {

enum LoggingSeverity {
  LS_VERBOSE,
  LS_ZORRO,
  LS_INFO,
  LS_WARNING,
  LS_ERROR,
  LS_NONE,
  INFO = LS_INFO,
  WARNING = LS_WARNING,
  LERROR = LS_ERROR,
  LZORRO = LS_ZORRO
};


#if RTC_LOG_ENABLED()
void msl_print(LoggingSeverity sev,const char* file, int line, const char* fmt, ...);
#else
inline void void msl_print(LoggingSeverity sev,const char* file, int line, const char* fmt, ...) {
  // Do nothing, shouldn't be invoked
}
#endif


}  // namespace rtc

#endif  // RTC_BASE_LOGGING_H_
