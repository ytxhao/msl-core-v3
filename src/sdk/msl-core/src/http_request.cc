#include "http_request.h"

#include <curl/curl.h>
#include <vector>

#include "rtc_base/time_utils.h"
// #include "rtc_base/logging.h"
// #include "rtc_base/net_helpers.h"

#include "zlog/zlog.h"
// #include "qos_metrics/qos.h"

namespace zorro {

struct HttpRequest::Context {
  std::atomic_bool cancel{false};
  std::string content{};
};

static size_t CurlWrite(void* ptr, size_t size, size_t nmemb, void* data) {
  auto context = (HttpRequest::Context*)data;
  if (context->cancel) {
    return 0;
  }
  auto count = size * nmemb;
  context->content.append((const char*)ptr, count);
  return count;
}

static int CurlProgress(void* clientp,
                        curl_off_t dltotal,
                        curl_off_t dlnow,
                        curl_off_t ultotal,
                        curl_off_t ulnow) {
  auto context = (HttpRequest::Context*)clientp;
  return context->cancel ? 1 : 0;
}

HttpRequest::HttpRequest() {
  thread_ = std::thread(std::bind(&HttpRequest::Run, this));
}

HttpRequest::~HttpRequest() {
  Cancel();
  {
    std::lock_guard<std::mutex> lock(mutex_);
    running_ = false;
    condition_.notify_one();
  }
  thread_.join();
}

void HttpRequest::Get(
    const std::string& url,
    const std::vector<std::pair<std::string, std::string>>& queries,
    int timeout,
    int retries,
    const std::function<void(const std::string& response)>& on_success,
    const std::function<void(int code, const std::string& error)>& on_error) {
  // create CURLU handle to append queries.
  CURLU* url_handle = curl_url();
  curl_url_set(url_handle, CURLUPART_URL, url.data(), 0);

  for (const auto& q : queries) {
    auto part = q.first + "=" + q.second;
    curl_url_set(url_handle, CURLUPART_QUERY, part.data(),
                 CURLU_URLENCODE | CURLU_APPENDQUERY);
  }
  // cancel pervious requests.
  Cancel();
  // create and store context.
  auto context = std::make_shared<Context>();
  std::atomic_store(&context_, context);
  // start request async.
  Schedule([=] () mutable {
    // init curl handle.
    auto curl = curl_easy_init();
    // get the complete url from CURLU handle.
    char* url_string = nullptr;
    curl_url_get(url_handle, CURLUPART_URL, &url_string, 0);
    curl_easy_setopt(curl, CURLOPT_URL, url_string);
    curl_url_cleanup(url_handle);
    curl_free(url_string);
    // disable signal handlers, for thread-safety.
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    // set request timeout.
    if (timeout) {
      curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout);
    }
    // TODO: set ca certificates and enable ssl verification.
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, context.get());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWrite);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, context.get());
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, CurlProgress);

#if defined(WEBRTC_IOS)
    // If LB configs have Ipv6 check, then IsIPv6Network result will be updated for each login
    if (!rtc::IsIPv6Stack() && !rtc::IsIPv6Network()) {
      curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    }
#else
    curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
#endif // WEBRTC_IOS

    // retry request if transient problems occur
    assert(retries >= 0 && retries < 10);
    CURLcode code = CURLE_AGAIN;
    int64_t start_time_ms = rtc::Time();
    int64_t t0_ms, t1_ms;
    for (int i = 0; running_ && code != CURLE_OK && code != CURLE_OPERATION_TIMEDOUT && i < retries + 1; i++) {
      if (i == 0) {
        ZLOG(ZLOG_TF, ZLOG_LI, true, "HTTP-Get")
            << "start " << std::make_pair("timeout", timeout) << std::endl;
      } else {
        ZLOG(ZLOG_ALL, ZLOG_LW, true, "HTTP-Get")
            << std::make_pair("retry", i)
            << std::make_pair("error", code)
            << std::endl;
        if (timeout > 0) {
          t1_ms = rtc::Time();
          timeout -= (t1_ms - t0_ms);
          if (timeout < 1000) {
            code = CURLE_OPERATION_TIMEDOUT;
            break;
          }
          curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout);
        }
      }

      t0_ms = rtc::Time();
      context->content.clear();
      code = curl_easy_perform(curl);
      // abort when receiving HTTP error codes.
      if (context->cancel || code == CURLE_HTTP_RETURNED_ERROR) {
        break;
      }
    }

    if (!context->cancel) {
      if (code != CURLE_OK) {
        ZLOG(ZLOG_ALL, ZLOG_LW, true, "HTTP-Get") << std::make_pair("error", code) << std::endl;
        on_error(code, curl_easy_strerror(code));
      } else {
        ZLOG(ZLOG_TF, ZLOG_LI, true, "HTTP-Get")
            << "responsed " << std::make_pair("time_ms", rtc::Time() - start_time_ms) << std::endl;
        // QOS_METRIC("HTTP-Get") << QOS_ELAPSED(rtc::Time() - start_time_ms);
        on_success(context->content);
      }
    } else {
        ZLOG(ZLOG_ALL, ZLOG_LW, true, "HTTP-Get")
            << "canceled " << std::make_pair("error", code) << std::endl;
    }
    curl_easy_cleanup(curl);
    context->content.clear();
  });
}

void HttpRequest::Cancel() {
  auto context = std::atomic_load(&context_);
  if (context) {
    context->cancel = true;
  }
}

void HttpRequest::Schedule(std::function<void()> task) {
  std::lock_guard<std::mutex> lock(mutex_);
  task_.swap(task);
  condition_.notify_one();
}

void HttpRequest::Run() {
  while (running_) {
    std::function<void()> task;
    {
      std::unique_lock<std::mutex> lock(mutex_);
      condition_.wait(lock, [this] { return !running_ || task_ != nullptr; });
      task.swap(task_);
    }
    if (task == nullptr) {
      break;
    }
    task();
  }
}

}  // namespace zorro
