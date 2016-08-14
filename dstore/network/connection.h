#ifndef DSTORE_NETWORK_CONNECTION_H_
#define DSTORE_NETWORK_CONNECTION_H_

#include <list>
#include "event_loop.h"
#include "socket.h"
#include "inet_addr.h"
#include "buffer.h"

namespace dstore
{
namespace network
{
class InetAddr;
class Message;
class Buffer;
class Connection
{
 public:
  Connection(int fd, const InetAddr &peer);
  void set_event(const Event &e);
  Event &get_event(void);
  Buffer &get_read_buffer(void);
  Buffer &get_write_buffer(void);
  void remove_write(void);
  void add_write(void);
  void remove_read(void);
  void add_read(void);
  Connection &operator=(const Connection &) = delete;
  Connection(const Connection &) = delete;
 private:
  Event e_;
  Socket socket_;
  InetAddr peer_;
  std::list<Message *> message_list_;
  Buffer read_buffer_;
  Buffer write_buffer_;
};
}  // end namespace network
}  // end namespace dstore
#endif  // DSTORE_NETWORK_CONNECTION_H_
