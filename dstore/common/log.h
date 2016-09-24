#ifndef DSTORE_COMMON_LOG_H_
#define DSTORE_COMMON_LOG_H_

#include <cstring>

namespace dstore
{
namespace common
{
class Log
{
 public:
  void log(const char *level, const char *filename, const int line, const char *format, ...);
  static Log &get_instance(void);
};
#define SHORT_FILE strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__
#define LOG_INSTANCE (::dstore::common::Log::get_instance())
#define LOG_DEBUG(message, args...) (LOG_INSTANCE.log("DEBUG", SHORT_FILE, __LINE__, message, ##args))
#define LOG_INFO(message, args...) (LOG_INSTANCE.log("INFO", SHORT_FILE, __LINE__, message, ##args))
#define LOG_WARN(message, args...) (LOG_INSTANCE.log("WARN", SHORT_FILE, __LINE__, message, ##args))
#define LOG_ERROR(message, args...) (LOG_INSTANCE.log("ERROR", SHORT_FILE, __LINE__, message, ##args))
}  // end namespace common
}  // end namespace dstore

#endif  // DSTORE_COMMON_LOG_H_
