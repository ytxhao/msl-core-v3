#ifndef MSL_MANAGER_H_
#define MSL_MANAGER_H_

#include <stdint.h>
#include <string>
#include "msl_common.h"
#include "msl_manager_interface.h"
namespace zorro {

class MslManager : public MslManagerInterface{
 public:
  static MslManager* GetInstance() {
    return instance_;
  }
  int InitLog(const LogConfig& log_config) override;
  std::string GetSDKVersion() override;
 private:
  static MslManager *instance_;
  LogConfig log_config_;
};

}
#endif // MSL_MANAGER_H_