#include "zlog.h"

// #include "rtc_base/net_helpers.h"
#include "rtc_base/logging.h"
#include "spdlog/spdlog.h"

#include <chrono>
#include <functional>
//#define TAG "zlog"
#define REPORT_RETRY 30
#define REPORT_TIMEOUT 30 * 1000
#define MAX_BUFFER_RECORD_COUNT 128

#define KB_ZORRO_URL "https://livehouse.ushow.media/track/zorro"

extern "C" {

void ZkbLogErrorMessage(const char* action, int code, const char* desc) {
  zorro::ZkbLog::Logger()->SendErrorMessage(action, code, desc);
}

void ZkbLogInfoMessage(const char* action, const char* msg) {
  zorro::ZkbLog::Logger()->SendInfoMessage(action, msg);
}

}

namespace {

size_t curl_write_func(char *buffer,
                       size_t size,
                       size_t nitems,
                       void *outstream) {
  return size * nitems;
}

}

// #define RTC_ZLOG(sev, file, line)           \
//   !rtc::LogMessage::IsNoop<::rtc::sev>() && \
//       RTC_LOG_FILE_LINE(::rtc::sev, file, line)
#define RTC_ZLOG(sev, file, line, tag, fmt, ...) rtc::msl_print(::rtc::sev, file, line, tag, fmt, ##__VA_ARGS__);

namespace zorro {

std::string ZlogKeyValue::GetPrefixLogLevel() {
  // RTC_DCHECK(msg_ != nullptr);
  switch (level_) {
    case ZLOG_LD: return "D: ";
    case ZLOG_LI: return "I: ";
    case ZLOG_LW: return "W: ";
    case ZLOG_LE: return "E: ";
    default:
      // RTC_DCHECK(0);
    break;
  }
  return "";
}

void ZlogKeyValue::Flush() {
  if (flush_) {
    return;
  }
  flush_ = true;

  if (type_mask_ & ZLOG_K) {
    if (msg_ != nullptr) {
      Append("msg", GetPrefixLogLevel() + msg_->str(), true);
      delete msg_;
      msg_ = nullptr;
    }
    ZkbLog::Logger()->SendMessage(ss_.str());
  }

  if ((type_mask_ & ZLOG_T) || (type_mask_ & ZLOG_F)) {
    const char* tag = (tag_ == nullptr ? "msl" : tag_);
    switch (level_) {
      case ZLOG_LD:
        if (type_mask_ & ZLOG_T) RTC_ZLOG(LS_VERBOSE, file_, line_, tag, t_f_ss_.str().c_str());
        if (type_mask_ & ZLOG_F) spdlog::debug("{}: {}", tag, t_f_ss_.str());
        break;
      case ZLOG_LI:
        if (type_mask_ & ZLOG_T) RTC_ZLOG(LS_INFO, file_, line_, tag, t_f_ss_.str().c_str());
        if (type_mask_ & ZLOG_F) spdlog::info("{}: {}", tag, t_f_ss_.str());
        break;
      case ZLOG_LW:
        if (type_mask_ & ZLOG_T) RTC_ZLOG(LS_WARNING, file_, line_, tag, t_f_ss_.str().c_str());
        if (type_mask_ & ZLOG_F) spdlog::warn("{}: {}", tag,t_f_ss_.str());
        break;
      case ZLOG_LE:
        if (type_mask_ & ZLOG_T) RTC_ZLOG(LS_ERROR, file_, line_, tag, t_f_ss_.str().c_str());
        if (type_mask_ & ZLOG_F) spdlog::error("{}: {}", tag, t_f_ss_.str());
        break;
      default:
        // RTC_DCHECK(0);
        break;
    }
  }
}


enum { MSG_SYNC_TIME = 0, MSG_LOOP_START };

ZkbLog *ZkbLog::instance_ = new ZkbLog();

ZkbLog::ZkbLog() : curl_(nullptr), loop_(true) {
  thread_ = std::thread(std::bind(&ZkbLog::Loop, this));
  curl_ = curl_easy_init();

  context_.curl_shared = curl_easy_init();
  context_.userId = "10086";
  context_.version = "0.0.0";
  context_.actionId = 0;
  context_.trackId = 0;
  context_.drop = 0;
}

void ZkbLog::newTrack(const std::string& uid, const std::string& version) {
  context_.userId = uid;
  context_.version = version;
  context_.actionId = 0;
  context_.drop = 0;
  context_.trackId = context_.trackId + 1;
}

int ZkbLog::ActionId() {
  std::lock_guard<std::mutex> lock(mutex_);
  int actionId_ = context_.actionId;
  context_.actionId = actionId_ + 1;
  return actionId_;
}

void ZkbLog::SendErrorMessage(const std::string& action, int code, const std::string& desc) {
  ZLOG(ZLOG_K, ZLOG_LE, false, action.c_str()) << std::make_pair("error", code)
                                               << std::make_pair("text", desc)
                                               << std::endl;
}

void ZkbLog::SendInfoMessage(const std::string& action, const std::string& msg) {
  ZLOG(ZLOG_K, ZLOG_LI, true, action.c_str()) << std::make_pair("msg", msg)
                                               << std::endl;
}

void ZkbLog::SendMessage(const std::string& queries) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (record_list_.size() > MAX_BUFFER_RECORD_COUNT) {
    record_list_.pop_front();
    context_.drop = context_.drop + 1;
  }
  record_list_.push_back(queries);
}

void ZkbLog::Loop() {
  while (loop_) {
    if (record_list_.size() <= 0) {
      std::this_thread::sleep_for(std::chrono::seconds(5));
      continue;
    }
    std::string query;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      query = record_list_.front();
      record_list_.pop_front();
    }
    bool ok = Post(query);
    std::this_thread::sleep_for(std::chrono::seconds(ok ? 1 : 10));
  }
}

bool ZkbLog::Post(std::string& query) {
  if (curl_ == nullptr) {
    curl_ = curl_easy_init();
  }

  std::string url = KB_ZORRO_URL;
  url = url + "?" + query;
  if (context_.drop > 0) {
    url += "&_r_drop_=" + std::to_string(context_.drop);
  }
  int count = record_list_.size();
  if (count > 0) {
    url += "&_r_job_count_=" + std::to_string(count);
  }

  curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());

  curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1L);
  curl_easy_setopt(curl_, CURLOPT_TIMEOUT_MS, REPORT_TIMEOUT);
  curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl_, CURLOPT_DNS_CACHE_TIMEOUT, 20*60L);

  curl_easy_setopt(curl_, CURLOPT_TCP_KEEPALIVE, 1L);
  curl_easy_setopt(curl_, CURLOPT_TCP_KEEPIDLE, 120L);
  curl_easy_setopt(curl_, CURLOPT_TCP_KEEPINTVL, 30L);

#if defined(WEBRTC_IOS)
  // If LB configs have Ipv6 check, then IsIPv6Network result will be updated for each login
  if (!rtc::IsIPv6Stack() && !rtc::IsIPv6Network()) {
    curl_easy_setopt(curl_, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
  }
#else
  curl_easy_setopt(curl_, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
#endif // WEBRTC_IOS

  // Donot use default fwrite, to avoid unused log print on iOS.
  curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, curl_write_func);

  bool ok = false;
  for (int i = 1; i <= REPORT_RETRY; i++) {
    CURLcode code = curl_easy_perform(curl_);
    if (code == CURLE_OK) {
      ok = true;
      break;
    }

    std::string error = curl_easy_strerror(code);
    char* output = curl_easy_escape(curl_, error.c_str(), 0);
    error = output;
    curl_free(output);
    std::string retryUrl =
        url + "&_r_retry_=" + std::to_string(i) + "&_r_err_=" + error;

    curl_easy_setopt(curl_, CURLOPT_URL, retryUrl.c_str());
    std::this_thread::sleep_for(std::chrono::seconds(10));
  }

  if (!ok) {
    curl_easy_cleanup(curl_);
    curl_ = nullptr;
  }
  return ok;
}
}  // namespace zorro
