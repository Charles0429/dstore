#include "tcp_server.h"
#include "errno_define.h"
#include "log.h"

using namespace dstore::common;
using namespace dstore::network;

int main(int argc, char **argv)
{
  TCPServer server("127.0.0.1", "8900", false);
  int ret = DSTORE_SUCCESS;
  if (DSTORE_SUCCESS != (ret = server.start())) {
    LOG_ERROR("can not start server, ret=%d", ret);
    return ret;
  }
  server.loop();
  return 0;
}
