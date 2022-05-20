#ifndef MSL_MANAGER_H_
#define MSL_MANAGER_H_

#include <stdint.h>
#include <string>
#include "msl_common.h"

namespace zorro {

class MslManager {
  public:
    int InitLog(const LogConfig& log_config);
    std::string GetSDKVersion();
  private:
    LogConfig log_config_;
};

}
#endif // MSL_MANAGER_H_