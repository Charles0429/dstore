#include "log.h"
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <sys/time.h>
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
    char time_buffer[80];
    char buffer[1024];
    va_list args;
	  timeval cur_time;
    size_t len = 0;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
	  gettimeofday(&cur_time, NULL);
	  len = strftime(time_buffer, 80, "%Y-%m-%d %H:%M:%S", localtime(&cur_time.tv_sec));
	  snprintf(time_buffer + len, 80, "%ld", cur_time.tv_usec);
    std::cout << level << ": " << time_buffer << " " << filename << ":" << line << " " << buffer << std::endl;
  }
}  // end namespace common
}  // end namespace dstore
