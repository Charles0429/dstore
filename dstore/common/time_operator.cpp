#include "time_operator.h"
#include <sys/time.h>

namespace dstore
{
namespace common
{
  int64_t get_milliseconds(void)
  {
    struct timeval tp;
    gettimeofday(&tp, nullptr);
    return tp.tv_sec * 1000 + tp.tv_usec / 1000;
  }
}  // end namespace common
}  // end namespace dstore
