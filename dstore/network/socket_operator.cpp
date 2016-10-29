#include "socket_operator.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "log.h"

namespace dstore
{
namespace network
{

int socket(int family, int socket_type, int protocol)
{
  int ret = ::socket(family, socket_type, protocol);
  if (-1 == ret) {
    LOG_WARN("create socket failed, family=%d, socket_type=%d, protocol=%d, errno=%d",
        family, socket_type, protocol, errno);
    return ret;
  }
  return ret;
}

int bind(int fd, const struct sockaddr *addr, int addr_len)
{
  int ret = ::bind(fd, addr, addr_len);
  if (-1 == ret) {
    LOG_WARN("bind fd failed, fd=%d, addrlen=%d, errno=%d", fd, addr_len, errno);
    return ret;
  }
  return ret;
}

int listen(int fd, int backlog)
{
  int ret = ::listen(fd, backlog);
  if (-1 == ret) {
    LOG_WARN("listen failed, fd=%d, backlog=%d, errno=%d", fd, backlog, errno);
    return ret;
  }
  return ret;
}
int accept(int fd, struct sockaddr *addr, int *addr_len)
{
  int ret = -1;
  socklen_t sock_len = static_cast<socklen_t>(*addr_len);
  ret = ::accept(fd, addr, &sock_len);
  if (-1 == ret) {
    LOG_WARN("fd=%d, addrlen=%d, ret= %d", fd, *addr_len, errno);
    return ret;
  }
  *addr_len = static_cast<int>(sock_len);
  return ret;
}

int set_reuseaddr(int fd)
{
  int on = 1;
  int ret = ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  if (-1 == ret) {
    LOG_WARN("set_reuseaddr failed, fd=%d, errno=%d", fd, errno);
    return ret;
  }
  return ret;
}

int set_nonblocking(int fd)
{
  int ret = ::fcntl(fd, F_SETFL, O_NONBLOCK);
  if (-1 == ret) {
    LOG_WARN("set nonblocking failed, fd=%d, errno=%d", fd, errno);
    return ret;
  }
  return ret;
}

ssize_t recv(int fd, void *buf, size_t nbytes)
{
  ssize_t nread = 0;
  size_t total_read = 0;
  char *ptr = static_cast<char *>(buf);
  while (total_read != nbytes) {
    nread = ::read(fd, ptr, nbytes - total_read);
    if (0 == nread) {
      return total_read;
    } else if (-1 == nread) {
      return total_read == 0 ? -1 : total_read;
    } else {
      total_read += nread;
      ptr += static_cast<size_t>(nread);
    }
  }
  return total_read;
}

ssize_t send(int fd, const void *buf, size_t nbytes)
{
  ssize_t nwrite = 0;
  size_t total_write = 0;
  const char *ptr = static_cast<const char *>(buf);

  while (total_write != nbytes) {
    nwrite = ::write(fd, ptr, nbytes - total_write);
    if (0 == nwrite) {
      return total_write;
    } else if (-1 == nwrite) {
      return nwrite == 0 ? -1 : nwrite;
    } else {
      total_write += nwrite;
      ptr += nwrite;
    }
  }
  return total_write;
}

int close(int fd)
{
  return ::close(fd);
}

int connect(int fd, const struct sockaddr *addr, int addrlen)
{
  return ::connect(fd, addr, addrlen);
}

bool is_connect_ok(int fd)
{
  int error = 0;
  unsigned int sz = sizeof(error);
  int ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, static_cast<void *>(&error), &sz);
  return !((ret < 0) || error);
}

}  // end namespace network
}  // end namespace dstore
