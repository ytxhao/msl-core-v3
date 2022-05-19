#ifndef MSL_MANAGER_H_
#define MSL_MANAGER_H_

#include <stdint.h>
#include <string>

namespace zorro {

struct LogConfig {
  LogConfig();
  LogConfig(int apiLevel,
            const std::string &osVersion,
            const std::string &manufacturer,
            const std::string &brand,
            const std::string &model,
            const std::string &buildFingerprint,
            const std::string &appId,
            const std::string &appVersion,
            const std::string &path,
            bool encrypt);
  LogConfig(const LogConfig& other);
  int apiLevel;
  std::string osVersion;
  std::string manufacturer;
  std::string brand;
  std::string model;
  std::string buildFingerprint;
  std::string appId;
  std::string appVersion;
  std::string path;
  bool encrypt;
};

class MslManager {
  public:
    int InitLog(const LogConfig& log_config);
  private:
    LogConfig log_config_;
};

}
#endif // MSL_MANAGER_H_