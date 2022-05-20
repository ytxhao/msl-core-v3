#ifndef ZORRO_ZLOG_H_
#define ZORRO_ZLOG_H_

#include <list>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

#include "third_party/curl/include/curl/curl.h"
#include "sdk/msl-core/src/msl_common.h"

#define ZLOG_T   0x01 // output to terminal
#define ZLOG_F   0x02 // output to file
#define ZLOG_K   0x04 // output to KB
#define ZLOG_TF  0x03 // output to teminal and file
#define ZLOG_KF  0x06 // output to KB and file
#define ZLOG_ALL 0x07 // output to all above

#define ZLOG_LD  0x00 // level debug
#define ZLOG_LI  0x01 // level info
#define ZLOG_LW  0x02 // level warn
#define ZLOG_LE  0x03 // level error

namespace zorro {

// @one_key_msg: 仅对ZLOG_K有效。这么做是因为KB期望尽可能复用key，以免消耗过多性能。
//               true, 将所有key-value整合成一个字符串，即key="msg", value=该字符串；
//               false，包含多个key-value。
#define ZLOG(type, level, one_key_msg, action) \
  zorro::ZlogKeyValue(type, level, nullptr, action, one_key_msg, __FILE__, __LINE__)

#define ZTLOG(type, level, tag, one_key_msg, action) \
  zorro::ZlogKeyValue(type, level, tag, action, one_key_msg, __FILE__, __LINE__)

// 标志一场log开始(比如ui进了房间),主要会影响actionId,trackId的统计,不影响log的上报
#define ZKBLOG_CONFIG(uid, version, proxy_host_name, proxy_port) ZkbLog::Logger()->newTrack(uid, version, proxy_host_name, proxy_port);


void InitZlog(const LogConfig& log_config);

void DeinitZlog();

class ZlogKeyValue;
class ZkbLog {
 public:
  ZkbLog();

  static ZkbLog* Logger() {
    return instance_;
  }

  struct Context {
    CURL* curl_shared;
    int actionId;
    int trackId;
    int drop;
    std::string userId;
    std::string version;
    std::string proxy_host_name;
    int proxy_port;
  };

 public:
  void newTrack(const std::string& uid, const std::string& version, const std::string& proxy_host_name, int proxy_port);
  void SendMessage(const std::string& queries);
  Context* GetContext() { return &context_; }
  int ActionId();
  void SendErrorMessage(const std::string& action, int code, const std::string& desc);
  void SendInfoMessage(const std::string& action, const std::string& msg);

 private:
  void Loop();
  bool Post(std::string& query);

 private:
  struct KbRecord {
    std::string header;
    std::string data;
  };
  CURL* curl_;
  std::mutex mutex_;
  std::thread thread_;
  std::list<std::string> record_list_;
  bool loop_;
  Context context_;
  static ZkbLog *instance_;
};

class ZlogKeyValue {
 public:
  ZlogKeyValue(int type_mask, int level, const char* tag, const char* action, bool one_key_msg, const char* file, int line)
      : type_mask_(type_mask),
        level_(level),
        tag_(tag),
        file_(file),
        line_(line),
        one_key_msg_(one_key_msg),
        msg_(nullptr),
        flush_(false) {
    if (type_mask & ZLOG_K) {
      auto context = ZkbLog::Logger()->GetContext();
      auto actionId = ZkbLog::Logger()->ActionId();
      curl_shared_ = context->curl_shared;

      Append("app", "msl", true);
      Append("ver", context->version, true);
      Append("appUid", context->userId, true);
      Append("trackId", context->trackId, true);
      Append("act", action, true);
      Append("actionId", actionId, true);
      Append("ts", time(NULL), true);
      std::thread::id thread_id = std::this_thread::get_id();
      ss_ << "&tid=" << thread_id;
    }

    if ((type_mask & ZLOG_T) || (type_mask & ZLOG_F)) {
      t_f_ss_ << action << " ";
    }
  }

  ~ZlogKeyValue() { Flush(); }

  ZlogKeyValue& Append(const char* k, const std::string& v, bool kb_fixed_key = false) {
    return Append(k, v.c_str(), kb_fixed_key);
  }

  ZlogKeyValue& Append(const char* k, const char* v, bool kb_fixed_key = false) {
    if (type_mask_ & ZLOG_K) {
      char* output = curl_easy_escape(curl_shared_, v, 0);
      if (!one_key_msg_ || kb_fixed_key) {
        if (ss_.str().length() > 0) {
          ss_ << "&";
        }
        ss_ << k << "=" << output;
      } else {
        if (msg_ == nullptr) {
          msg_ = new std::stringstream();
        }
        *msg_ << k << "=" << v << ", ";
      }
      curl_free(output);
    }
    if (((type_mask_ & ZLOG_T) || (type_mask_ & ZLOG_F)) && !kb_fixed_key) {
      t_f_ss_ << k << "=" << v << ", ";
    }
    return *this;
  }

  template <class T>
  ZlogKeyValue& Append(const char* k, T v, bool kb_fixed_key = false) {
    if (type_mask_ & ZLOG_K) {
      if (!one_key_msg_ || kb_fixed_key) {
        if (ss_.str().length() > 0) {
          ss_ << "&";
        }
        ss_ << k << "=" << v;
      } else {
        if (msg_ == nullptr) {
          msg_ = new std::stringstream();
        }
        *msg_ << k << "=" << v << ", ";
      }
    }
    if (((type_mask_ & ZLOG_T) || (type_mask_ & ZLOG_F)) && !kb_fixed_key) {
      t_f_ss_ << k << "=" << v << ", ";
    }
    return *this;
  }

  ZlogKeyValue& Append(const char* v) {
    if (type_mask_ & ZLOG_K) {
      if (msg_ == nullptr) {
        msg_ = new std::stringstream();
      }
      *msg_ << v;
    }
    if ((type_mask_ & ZLOG_T) || (type_mask_ & ZLOG_F)) {
      t_f_ss_ << v;
    }
    return *this;
  }

  template <class T>
  ZlogKeyValue& Append(T v) {
    if (type_mask_ & ZLOG_K) {
      if (msg_ == nullptr) {
        msg_ = new std::stringstream();
      }
      *msg_ << v;
    }
    if ((type_mask_ & ZLOG_T) || (type_mask_ & ZLOG_F)) {
      t_f_ss_ << v;
    }
    return *this;
  }

  ZlogKeyValue& operator<<(const std::pair<const char*, const char*>& kv) {
    return Append(kv.first, kv.second, false);
  }

  ZlogKeyValue& operator<<(const std::pair<const char*, const std::string&>& kv) {
    return Append(kv.first, kv.second, false);
  }

  template <class T>
  ZlogKeyValue& operator<<(const std::pair<const char*, T>& kv) {
    return Append(kv.first, kv.second, false);
  }

  ZlogKeyValue& operator<<(const char* msg) { return Append(msg); }
  ZlogKeyValue& operator<<(const std::string& msg) { return Append(msg.c_str()); }
  template <class T>
  ZlogKeyValue& operator<<(T v) { return Append(v); }

  typedef std::basic_ostream<char, std::char_traits<char>> StdOStream;
  typedef StdOStream& (*KBFlush)(StdOStream&);
  void operator<<(KBFlush flush) { Flush(); }

 private:
  std::string GetPrefixLogLevel();
  void Flush();

 private:
  int type_mask_;
  int level_;
  const char* tag_;
  const char* file_;
  int line_;
  bool one_key_msg_;
  CURL* curl_shared_;
  std::stringstream t_f_ss_; // for terminal/file output
  std::stringstream ss_;
  std::stringstream* msg_;
  bool flush_;
};

}  // namespace zorro

#endif // ZORRO_ZLOG_H_
