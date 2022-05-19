

#include "msl_manager.h"
namespace zorro {

LogConfig::LogConfig()
    : apiLevel(0),
      osVersion(""),
      manufacturer(""),
      brand(""),
      model(""),
      buildFingerprint(""),
      appId(""),
      appVersion(""),
      path(""),
      encrypt(true) {}

LogConfig::LogConfig(int apiLevel,
                     const std::string &osVersion,
                     const std::string &manufacturer,
                     const std::string &brand,
                     const std::string &model,
                     const std::string &buildFingerprint,
                     const std::string &appId,
                     const std::string &appVersion,
                     const std::string &path,
                     bool encrypt)
    : apiLevel(apiLevel),
      osVersion(osVersion),
      manufacturer(manufacturer),
      brand(brand),
      model(model),
      buildFingerprint(buildFingerprint),
      appId(appId),
      appVersion(appVersion),
      path(path),
      encrypt(encrypt) {}

LogConfig::LogConfig(const LogConfig& other) = default;

int MslManager::InitLog(const LogConfig& log_config) {
    return 1;
}

}
