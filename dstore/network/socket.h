#ifndef DSTORE_NETWORK_SOCKET_H_
#define DSTORE_NETWORK_SOCKET_H_

#include "inet_addr.h"

namespace dstore
{
namespace network
{
class ListenSocket
{
 public:
  ListenSocket(void);
  explicit ListenSocket(int listen_fd);
  ~ListenSocket(void);
  void set_listen_fd(int listen_fd);
  int get_listen_fd(void);
  int socket(const InetAddr &addr);
  int bind(const InetAddr &addr);
  int listen(int backlog);
  int accept(int &fd, InetAddr *addr);
  int set_reuseaddr(void);
  int set_nonblocking(void);
  ListenSocket &operator=(const ListenSocket &) = delete;
  ListenSocket(const ListenSocket &) = delete;
 private:
  int listen_fd_;
};

class Socket
{
 public:
  Socket(void);
  explicit Socket(int fd);
  ~Socket(void);
  void set_fd(int fd);
  Socket &operator=(const Socket &) = delete;
  Socket(const Socket &) = delete;
 private:
  int fd_;
};
}  // end namespace network
}  // end namespace dstore

#endif  // DSTORE_NETWORK_SOCKET_H_
