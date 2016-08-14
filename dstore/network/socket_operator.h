#ifndef DSTORE_NETWORK_SOCKET_OPERATOR_H_
#define DSTORE_NETWORK_SOCKET_OPERATOR_H_

#include <sys/types.h>
#include <sys/socket.h>

namespace dstore
{
namespace network
{
  int socket(int ai_family, int socket_type, int protocol);
  int bind(int fd, const struct sockaddr *addr, int addr_len);
  int listen(int fd, int backlog);
  int accept(int fd, struct sockaddr *addr, int *addr_len);
  int set_reuseaddr(int fd);
  int set_nonblocking(int fd);
  ssize_t recv(int fd, void *buf, size_t size);
  ssize_t send(int fd, const void *buf, size_t size);
  int close(int fd);
}  // end namespace network
}  // end namespace dstore

#endif  // DSTORE_NETWORK_SOCKET_OPERATOR_H_
