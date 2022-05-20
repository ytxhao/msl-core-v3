#include "msl_common.h"

namespace zorro {

LogConfig::LogConfig()
    : api_level(0),
      proxy_port(0),
      proxy_host_name(""),
      os_version(""),
      manufacturer(""),
      brand(""),
      model(""),
      build_fingerprint(""),
      app_id(""),
      app_version(""),
      msl_version(""),
      user_id(""),
      path(""),
      encrypt(true) {}

LogConfig::LogConfig(int api_level,
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
                     bool encrypt)
    : api_level(api_level),
      proxy_port(proxy_port),
      proxy_host_name(proxy_host_name),
      os_version(os_version),
      manufacturer(manufacturer),
      brand(brand),
      model(model),
      build_fingerprint(build_fingerprint),
      app_id(app_id),
      app_version(app_version),
      msl_version(msl_version),
      user_id(user_id),
      path(path),
      encrypt(encrypt) {}

LogConfig::LogConfig(const LogConfig& other) = default;

}