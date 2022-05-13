#ifndef HTTP_REQUEST_H_
#define HTTP_REQUEST_H_

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace zorro {

class HttpRequest {
 public:
  HttpRequest();
  virtual ~HttpRequest();

 public:
  virtual void Get(
      const std::string& url,
      const std::vector<std::pair<std::string, std::string>>& queries,
      int timeout,
      int retries,
      const std::function<void(const std::string& response)>& on_success,
      const std::function<void(int code, const std::string& error)>& on_error);
  virtual void Cancel();

  struct Context;

 private:
  void Schedule(std::function<void()> task);
  void Run();

 private:
  std::shared_ptr<Context> context_;
  std::mutex mutex_;
  std::thread thread_;
  std::condition_variable condition_;
  std::function<void()> task_;
  std::atomic_bool running_{true};
};

}  // namespace zorro

#endif  // HTTP_REQUEST_H_
