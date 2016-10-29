#include "socket.h"
#include <errno.h>
#include "errno_define.h"
#include "socket_operator.h"
#include "log.h"

using namespace dstore::network;

ListenSocket::ListenSocket(void)
  : listen_fd_(-1)
{
}

ListenSocket::ListenSocket(int listen_fd)
  : listen_fd_(listen_fd)
{
}

ListenSocket::~ListenSocket(void)
{
}

void ListenSocket::set_listen_fd(int listen_fd)
{
  listen_fd_ = listen_fd;
}

int ListenSocket::get_listen_fd(void)
{
  return listen_fd_;
}

int ListenSocket::socket(const InetAddr &addr)
{
  int ret = DSTORE_SUCCESS;
  int listen_fd = dstore::network::socket(addr.get_family(), addr.get_socket_type(), addr.get_protocol());
  if (-1 == listen_fd) {
    LOG_WARN("create listen socket failed");
    ret = DSTORE_LISTEN_ERROR;
    return ret;
  }
  listen_fd_ = listen_fd;
  return ret;
}

int ListenSocket::bind(const InetAddr &addr)
{
  int ret = DSTORE_SUCCESS;
  ret = dstore::network::bind(listen_fd_, addr.get_addr(), *addr.get_addr_len());
  if (-1 == ret) {
    LOG_WARN("bind listen socket failed, fd=%d", listen_fd_);
    ret = DSTORE_BIND_ERROR;
    return ret;
  }
  return ret;
}

int ListenSocket::listen(int backlog)
{
  int ret = DSTORE_SUCCESS;
  ret = dstore::network::listen(listen_fd_, backlog);
  if (-1 == ret) {
    LOG_WARN("listen socket failed, fd=%d", listen_fd_);
    ret = DSTORE_LISTEN_ERROR;
    return ret;
  }
  return ret;
}

int ListenSocket::accept(int *fd, InetAddr *addr)
{
  int ret = DSTORE_SUCCESS;
  *fd = dstore::network::accept(listen_fd_, addr->get_addr(), addr->get_addr_len());
  if (-1 == *fd) {
    LOG_WARN("accept failed, listen_fd=%d", listen_fd_);
    ret = DSTORE_ACCEPT_ERROR;
    return ret;
  }
  return ret;
}

int ListenSocket::set_reuseaddr(void)
{
  int ret = DSTORE_SUCCESS;
  ret = dstore::network::set_reuseaddr(listen_fd_);
  if (-1 == ret) {
    LOG_WARN("set reuseaddr failed, listen_fd=%d", listen_fd_);
    ret = DSTORE_SET_SOCKET_ERROR;
    return ret;
  }
  return ret;
}

int ListenSocket::set_nonblocking(void)
{
  int ret = DSTORE_SUCCESS;
  ret = dstore::network::set_nonblocking(listen_fd_);
  if (-1 == ret) {
    LOG_WARN("set nonblocking failed, listen_fd=%d", listen_fd_);
    ret = DSTORE_SET_SOCKET_ERROR;
    return ret;
  }
  return ret;
}

Socket::Socket(void)
  : fd_(-1)
{
}

Socket::Socket(int fd)
  : fd_(fd)
{
}

Socket::~Socket(void)
{
}

void Socket::set_fd(int fd)
{
  fd_ = fd;
}

int Socket::get_fd(void)
{
  return fd_;
}

int Socket::socket(const InetAddr &addr)
{
  int ret = DSTORE_SUCCESS;
  int fd = dstore::network::socket(addr.get_family(), addr.get_socket_type(), addr.get_protocol());
  if (-1 == fd) {
    LOG_WARN("create listen socket failed");
    ret = DSTORE_LISTEN_ERROR;
    return ret;
  }
  fd_ = fd;
  return ret;
}

int Socket::connect(const InetAddr &addr)
{
  int ret = DSTORE_SUCCESS;
  ret = dstore::network::connect(fd_, addr.get_addr(), *addr.get_addr_len());
  if (-1 == ret) {
    if (errno == EINPROGRESS) {
      ret = DSTORE_CONNECT_IN_PROGRESS;
    } else {
      ret = DSTORE_CONNECT_ERROR;
    }
  }
  return ret;
}

int Socket::set_nonblocking(void)
{
  int ret = DSTORE_SUCCESS;
  ret = dstore::network::set_nonblocking(fd_);
  if (-1 == ret) {
    LOG_WARN("set non blocking failed, fd_=%d", fd_);
    ret = DSTORE_SET_SOCKET_ERROR;
    return ret;
  }
  return ret;
}

void Socket::close(void)
{
  dstore::network::close(fd_);
}

bool Socket::is_connect_ok(void)
{
  return dstore::network::is_connect_ok(fd_);
}
