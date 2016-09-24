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
  enum Status
  {
    INVALID = 0,
    ACTIVE,
    SHUTDOWN_READ,
    SHUTDOWN_WRITE,
  };
  Connection(const Event &e, const InetAddr &peer);
  int init(EventLoop *loop);
  void set_event(const Event &e);
  Event &get_event(void);
  Buffer &get_read_buffer(void);
  Buffer &get_write_buffer(void);
  Socket &get_socket(void);
  int remove_write(void);
  int add_write(void);
  int remove_read(void);
  int add_read(void);
  void add_message(Message *);
  void remove_message(Message *);
  void set_status(const Connection::Status s);
  Connection::Status get_status(void);
  void close(void);
  bool pending_write(void);
  std::list<Message *> &get_message_list(void);
  Connection &operator=(const Connection &) = delete;
  Connection(const Connection &) = delete;
 private:
  Status status_;
  Event e_;
  EventLoop *loop_;
  Socket socket_;
  InetAddr peer_;
  std::list<Message *> message_list_;
  Buffer read_buffer_;
  Buffer write_buffer_;
};
}  // end namespace network
}  // end namespace dstore
#endif  // DSTORE_NETWORK_CONNECTION_H_
