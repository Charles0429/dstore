#include "tcp_server.h"
#include "errno_define.h"
#include "log.h"
#include "epoll.h"
#include "connection.h"
#include "buffer.h"

using namespace std::placeholders;
using namespace dstore::network;
TCPServer::TCPServer(void)
  : loop_(), listener_(), connection_map_(),
  next_connection_id_(0)
{
}

TCPServer::TCPServer(const char *host, const char *port, const bool is_ipv6)
  : host_(host), port_(port), is_ipv6_(is_ipv6), loop_(), listener_(),
    connection_map_(), next_connection_id_(0)
{
}

TCPServer::~TCPServer(void)
{
}

int TCPServer::start(void)
{
  int ret = DSTORE_SUCCESS;
  if (DSTORE_SUCCESS != (ret = listener_.set_addr(host_.c_str(), port_.c_str(), is_ipv6_))) {
    LOG_ERROR("listener set addr failed, host=%s, port=%s, is_ipv6=%d, ret=%d",
        host_.c_str(), port_.c_str(), is_ipv6_, ret);
    return ret;
  }
  if (DSTORE_SUCCESS != (ret = listener_.start())) {
    LOG_ERROR("listener start failed, ret=%d", ret);
    return ret;
  }
  // init event loop
  EventPollAPI *poll_api = new EpollAPI(&loop_);
  if (DSTORE_SUCCESS != (ret = loop_.init(poll_api))) {
    LOG_ERROR("init event loop failed, ret=%d", ret);
    return ret;
  }
  // register the event
  Event &e = listener_.get_listen_event();
  if (DSTORE_SUCCESS != (ret = loop_.register_event(e))) {
    LOG_ERROR("register listen event failed, ret=%d", ret);
    return ret;
  }

  // set connection callback
  listener_.set_new_connection_callback(std::bind(&TCPServer::on_connect, this, _1, _2));
  return ret;
}

void TCPServer::loop(void)
{
  loop_.loop();
}

int TCPServer::on_connect(int fd, InetAddr *addr)
{
  int ret = DSTORE_SUCCESS;
  Event e;
  e.fd = fd;
  e.type = Event::kEventRead;
  e.args = reinterpret_cast<void *>(next_connection_id_);
  e.read_cb = std::bind(&TCPServer::on_read, this, _1, _2, _3);
  e.write_cb = std::bind(&TCPServer::on_write, this, _1, _2, _3);
  if (DSTORE_SUCCESS != (ret = loop_.register_event(e))) {
    LOG_WARN("register event failed, fd=%d, type=%d, ret=%d", e.fd, e.type, ret);
    return ret;
  }
  Connection *connection = new Connection(fd, *addr);
  connection_map_[next_connection_id_++] = connection;
  return ret;
}

int TCPServer::on_read(int fd, int type, void *args)
{
  int ret = DSTORE_SUCCESS;
  int64_t connection_id = reinterpret_cast<int64_t>(args);
  Connection *connection = connection_map_[connection_id];
  Buffer &read_buffer = connection->get_read_buffer();
  const int nbytes = 1024;
  ssize_t read_bytes = read_buffer.read_fd(fd, nbytes);
  if (0 == read_bytes) {
    // close the fd
    return ret;
  }
  bool has_new_message = false;
  if (DSTORE_SUCCESS != (ret = message_decoder_(connection, has_new_message))) {
    LOG_WARN("decode message failed, ret=%d", ret);
    return ret;
  }
  return new_message_callback_(connection);
}

int TCPServer::on_write(int fd, int type, void *args)
{
  int ret = DSTORE_SUCCESS;
  int64_t connection_id = reinterpret_cast<int64_t>(args);
  Connection *connection = connection_map_[connection_id];
  Buffer &write_buffer = connection->get_write_buffer();
  ssize_t write_bytes = write_buffer.write_fd(fd);
  if (-1 == write_bytes) {
    // handle error
    return ret;
  }
  size_t writeable_bytes = write_buffer.get_write_bytes();
  if (0 == writeable_bytes) {
    connection->remove_write();
    ret = loop_.modify_event(connection->get_event());
  } else if (writeable_bytes > 0) {
    // do not modify the event
    // just let it in loop again
  }
  return ret;
}
