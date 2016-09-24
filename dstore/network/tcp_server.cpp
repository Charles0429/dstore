#include "tcp_server.h"
#include <cassert>
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
  connection_map_(), next_connection_id_(0), message_decoder_(),
  new_message_callback_()
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

  LOG_INFO("listener started");
  // set connection callback
  listener_.set_new_connection_callback(std::bind(&TCPServer::on_connect, this, _1, _2));
  return ret;
}

int TCPServer::loop(void)
{
  int ret = DSTORE_SUCCESS;
  if (DSTORE_SUCCESS != (ret = loop_.loop())) {
    LOG_WARN("loop failed, ret=%d", ret);
    return ret;
  }
  return ret;
}

void TCPServer::set_message_decode_callback(const MessageDecodeCallback &message_decode)
{
  message_decoder_ = message_decode;
}

void TCPServer::set_new_message_callback(const NewMessageCallback &new_message)
{
  new_message_callback_ = new_message;
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
  Connection *connection = new Connection(e, *addr);
  connection_map_[next_connection_id_++] = connection;
  if (DSTORE_SUCCESS != (ret = connection->init(&loop_))) {
    LOG_WARN("init connection failed, ret=%d", ret);
    return ret;
  }
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
    if (DSTORE_SUCCESS != (ret = on_close(connection_id))) {
      LOG_WARN("handle close failed, ret=%d", ret);
      return ret;
    }
    return ret;
  }
  if (DSTORE_SUCCESS != (ret = message_decoder_(connection))) {
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
    remove_connection(connection_id);
    return ret;
  }
  size_t need_write_bytes = write_buffer.get_need_write_bytes();
  if (0 == need_write_bytes) {
    if (DSTORE_SUCCESS != (ret = connection->remove_write())) {
      LOG_WARN("remove write event from connection_id=%d failed, ret=%d", connection_id, ret);
      return ret;
    }
    if (Connection::SHUTDOWN_READ == connection->get_status()) {
      if (DSTORE_SUCCESS != (ret = on_close(connection_id))) {
        LOG_WARN("handle close connection failed, ret=%d", ret);
        return ret;
      }
    }
  } else if (need_write_bytes > 0) {
    // do not modify the event
    // just let it in loop again
  }
  return ret;
}

void TCPServer::remove_connection(const int64_t connection_id)
{
  Connection *conn = connection_map_[connection_id];
  conn->close();
  delete conn;
  connection_map_.erase(connection_id);
}

int TCPServer::set_connection_status(const int64_t connection_id, const Connection::Status status)
{
  int ret = DSTORE_SUCCESS;
  Connection *conn = connection_map_[connection_id];
  if (status == Connection::SHUTDOWN_READ) {
    if (DSTORE_SUCCESS != (ret = conn->remove_read())) {
      LOG_WARN("remove connection read event failed, ret=%d", ret);
      return ret;
    }
    conn->set_status(Connection::SHUTDOWN_READ);
  } else if (status == Connection::SHUTDOWN_WRITE) {
    if (DSTORE_SUCCESS != (ret = conn->remove_write())) {
      LOG_WARN("remove connection write event failed, ret=%d", ret);
      return ret;
    }
    conn->set_status(Connection::SHUTDOWN_WRITE);
  }
  return ret;
}

int TCPServer::on_close(const int64_t connection_id)
{
  int ret = DSTORE_SUCCESS;
  Connection *conn = connection_map_[connection_id];
  assert(conn != nullptr);
  bool has_data_to_write = conn->pending_write();
  if (!has_data_to_write) {
    remove_connection(connection_id);
  } else {
    set_connection_status(connection_id, Connection::SHUTDOWN_READ);
  }
  return ret;
}
