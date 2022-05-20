#ifndef MSL_MANAGER_INTERFACE_H_
#define MSL_MANAGER_INTERFACE_H_

#include <string>
#include <vector>

#include "msl_common.h"
namespace zorro {
class MslManagerInterface {
 public:
  virtual int InitLog(const LogConfig& log_config) = 0;
  virtual std::string GetSDKVersion() = 0;
};

} // namespace zorro

#endif // MSL_MANAGER_INTERFACE_H_