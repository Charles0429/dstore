#include "log.h"
#include <cstdarg>
#include <chrono>
#include <iostream>

namespace dstore
{
namespace common
{
  Log &Log::get_instance(void)
  {
    static Log log;
    return log;
  }

  void Log::log(const char *level, const char *filename, const int line, const char *format, ...)
  {
    char buffer[1024];
    std::chrono::time_point<std::chrono::system_clock> now;
    now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    std::cerr << level << ": " << std::ctime(&now_time) << " " << filename << " " << line << " " << buffer << std::endl;
  }
}  // end namespace common
}  // end namespace dstore
