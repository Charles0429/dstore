#include "socket_operator.h"
#include <unistd.h>
#include <fcntl.h>

namespace dstore
{
namespace network
{

int socket(int family, int socket_type, int protocol)
{
  return ::socket(family, socket_type, protocol);
}

int bind(int fd, const struct sockaddr *addr, int addr_len)
{
  return ::bind(fd, addr, addr_len);
}

int listen(int fd, int backlog)
{
  return ::listen(fd, backlog);
}
int accept(int fd, struct sockaddr *addr, int *addr_len)
{
  int ret = -1;
  socklen_t sock_len = static_cast<socklen_t>(*addr_len);
  ret = ::accept(fd, addr, &sock_len);
  *addr_len = static_cast<int>(sock_len);
  return ret;
}

int set_reuseaddr(int fd)
{
  int on = 1;
  return ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
}

int set_nonblocking(int fd)
{
  return ::fcntl(fd, F_SETFL, O_NONBLOCK);
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
      return -1;
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
      return -1;
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

}  // end namespace network
}  // end namespace dstore
