#include "tcp_connector.h"
#include "errno_define.h"
#include "log.h"

using namespace std::placeholders;
using namespace dstore::common;
using namespace dstore::network;

TCPConnector::TCPConnector(void)
  : addr_(), socket_()
{
}

int TCPConnector::set_addr(const char *host, const char *port, const bool is_ipv6)
{
  int ret = DSTORE_SUCCESS;
  const bool is_ai_passive = false;
  if (DSTORE_SUCCESS != (ret = addr_.set_addr(host, port, is_ipv6, is_ai_passive))) {
    LOG_WARN("set addr failed, host=%s, port=%s, is_ipv6=%d, ret=%d", host, port, is_ipv6, ret);
    return ret;
  }
  return ret;
}

int TCPConnector::start(void)
{
  int ret = DSTORE_SUCCESS;
  if (DSTORE_SUCCESS != (ret = socket_.socket(addr_))) {
    LOG_WARN("create socket failed, ret=%d", ret);
    return ret;
  }
  if (DSTORE_SUCCESS != (ret = socket_.set_nonblocking())) {
    LOG_WARN("set socket nonblocking failed, ret=%d", ret);
    return ret;
  }
  return ret;
}

int TCPConnector::connect(void)
{
  int ret = DSTORE_SUCCESS;
  if (DSTORE_SUCCESS != (ret = socket_.connect(addr_))) {
    if (DSTORE_CONNECT_IN_PROGRESS != ret) {
      LOG_WARN("connect failed, ret=%d", ret);
    }
    return ret;
  }
  return ret;
}

int TCPConnector::get_socket_fd(void)
{
  return socket_.get_fd();
}

InetAddr &TCPConnector::get_server_addr(void)
{
  return addr_;
}
