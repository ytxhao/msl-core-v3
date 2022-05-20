
#include "version.h"
#include "msl_manager.h"
#include "zlog/zlog.h"

namespace zorro {

int MslManager::InitLog(const LogConfig& log_config) {
  InitZlog(log_config);
  return 1;
}

std::string MslManager::GetSDKVersion() {
  return MSL_VERSION;
}

}
