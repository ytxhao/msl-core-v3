#ifndef MSL_COMMON_H_
#define MSL_COMMON_H_
#include <stdint.h>
#include <string>
namespace zorro {

struct LogConfig {
  LogConfig();
  LogConfig(int api_level,
            int proxy_port,
            const std::string &proxy_host_name,
            const std::string &os_version,
            const std::string &manufacturer,
            const std::string &brand,
            const std::string &model,
            const std::string &build_fingerprint,
            const std::string &app_id,
            const std::string &app_version,
            const std::string &msl_version,
            const std::string &user_id,
            const std::string &path,
            bool encrypt);
  LogConfig(const LogConfig& other);
  int api_level;
  int proxy_port;
  std::string proxy_host_name;
  std::string os_version;
  std::string manufacturer;
  std::string brand;
  std::string model;
  std::string build_fingerprint;
  std::string app_id;
  std::string app_version;
  std::string msl_version;
  std::string user_id;
  std::string path;
  bool encrypt;
};

}

#endif // MSL_COMMON_H_