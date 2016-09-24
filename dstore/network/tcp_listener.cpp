#include "tcp_listener.h"
#include "errno_define.h"
#include "log.h"
#include <iostream>

using namespace std::placeholders;
using namespace dstore::common;
using namespace dstore::network;

TCPListener::TCPListener(void)
  : addr_(), socket_(), e_(), on_connect_()
{
}

TCPListener::~TCPListener(void)
{
}

int TCPListener::set_addr(const char *host, const char *port, bool is_ipv6)
{
  int ret = DSTORE_SUCCESS;
  const bool is_ai_passive = true;
  ret = addr_.set_addr(host, port, is_ipv6, is_ai_passive);
  if (DSTORE_SUCCESS != ret) {
    LOG_WARN("set addr failed, host=%s, port=%s, is_ipv6=%d, ret=%d", host, port, is_ipv6, ret);
    return ret;
  }
  return ret;
}

void TCPListener::set_new_connection_callback(NewConnectionCallback on_connect)
{
  on_connect_ = on_connect;
}

Event &TCPListener::get_listen_event(void)
{
  return e_;
}

int TCPListener::start(void)
{
  int ret = DSTORE_SUCCESS;
  if (DSTORE_SUCCESS != (ret = socket_.socket(addr_))) {
    LOG_WARN("create socket failed, ret=%d", ret);
    return ret;
  }
  if (DSTORE_SUCCESS != (ret = socket_.set_reuseaddr())) {
    LOG_WARN("set reuseaddr failed, ret=%d", ret);
    return ret;
  }
  if (DSTORE_SUCCESS != (ret = socket_.set_nonblocking())) {
    LOG_WARN("set non blocking failed, ret=%d", ret);
    return ret;
  }
  if (DSTORE_SUCCESS != (ret = socket_.bind(addr_))) {
    LOG_WARN("bind failed, ret=%d", ret);
    return ret;
  }
  if (DSTORE_SUCCESS != (ret = socket_.listen(1024))) {
    LOG_WARN("listen failed, ret=%d", ret);
    return ret;
  }
  e_.fd = socket_.get_listen_fd();
  e_.type = Event::kEventRead;
  e_.args = nullptr;
  e_.read_cb = std::bind(&TCPListener::accept, this, _1, _2, _3);
  LOG_INFO("start listening, listen_fd=%d", e_.fd, e_.type);
  return ret;
}

int TCPListener::accept(int fd, int type, void *args)
{
  int ret = DSTORE_SUCCESS;
  int new_fd = -1;
  InetAddr addr;
  if (DSTORE_SUCCESS != (ret = socket_.accept(&new_fd, &addr))) {
    LOG_WARN("accept failed, ret=%d", ret);
    return ret;
  }
  return on_connect_(new_fd, &addr);
}
