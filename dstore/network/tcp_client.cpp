#include "tcp_client.h"
#include <cassert>
#include "epoll.h"
#include "errno_define.h"
#include "log.h"

using namespace std::placeholders;
using namespace dstore::network;

TCPClient::TCPClient(void)
  : loop_(), connection_map_(), next_connection_id_(0), message_decoder_(),
  new_message_callback_(), connect_complete_()
{
}

int TCPClient::start(void)
{
  int ret = DSTORE_SUCCESS;
  EventPollAPI *poll_api = new EpollAPI(&loop_);
  if (DSTORE_SUCCESS != (ret = loop_.init(poll_api))) {
    LOG_ERROR("init loop failed, ret=%d", ret);
    return ret;
  }
  return ret;
}

int TCPClient::connect(const char *host, const char *port, const bool is_ipv6)
{
  int ret = DSTORE_SUCCESS;
  if (DSTORE_SUCCESS != (ret = connector_.set_addr(host, port, is_ipv6))) {
    LOG_WARN("connector set addr failed, host=%s, port=%s, is_ipv6=%d, ret=%d",
        host, port, is_ipv6, ret);
    return ret;
  }

  if (DSTORE_SUCCESS != (ret = connector_.start())) {
    LOG_WARN("connector start failed, ret=%d", ret);
    return ret;
  }

  if (DSTORE_SUCCESS != (ret = connector_.connect())) {
    if (DSTORE_CONNECT_IN_PROGRESS != ret) {
      LOG_WARN("connect failed, ret=%d", ret);
      return ret;
    }
  }

  std::shared_ptr<Event> e(new Event());
  Connection::Status status = DSTORE_SUCCESS == ret ? Connection::ACTIVE : Connection::CONNECTING;
  e->fd = connector_.get_socket_fd();
  e->type = DSTORE_SUCCESS == ret ? Event::kEventRead : Event::kEventRead | Event::kEventWrite;
  e->args = reinterpret_cast<void *>(next_connection_id_);
  e->read_cb = std::bind(&TCPClient::on_read, this, _1, _2, _3);
  e->write_cb = std::bind(&TCPClient::on_write, this, _1, _2, _3);

  if (DSTORE_SUCCESS != (ret = loop_.register_event(e))) {
    LOG_WARN("register event failed, ret=%d", ret);
    return ret;
  }

  std::shared_ptr<Connection> conn(new Connection(connector_.get_server_addr(), e));
  if (DSTORE_SUCCESS != (ret = conn->init(&loop_))) {
    LOG_WARN("init connection failed, ret=%d", ret);
    return ret;
  }
  conn->set_status(status);
  connection_map_[next_connection_id_++] = conn;
  return ret;
}

int TCPClient::loop(void)
{
  int ret = DSTORE_SUCCESS;
  if (DSTORE_SUCCESS != (ret = loop_.loop())) {
    LOG_WARN("loop failed, ret=%d", ret);
    return ret;
  }
  return ret;
}

void TCPClient::set_message_decode_callback(const MessageDecodeCallback &message_decode)
{
  message_decoder_ = message_decode;
}

void TCPClient::set_new_message_callback(const NewMessageCallback &new_message)
{
  new_message_callback_ = new_message;
}

void TCPClient::set_connect_complete_callback(const ConnectCompeleteCallback &connect_complete)
{
  connect_complete_ = connect_complete;
}

int TCPClient::on_read(int fd, int type, void *args)
{
  int ret = DSTORE_SUCCESS;
  const int64_t connection_id = reinterpret_cast<int64_t>(args);
  std::shared_ptr<Connection> conn = connection_map_[connection_id];
  Buffer &read_buffer = conn->get_read_buffer();
  const int nbytes = 1024;
  ssize_t read_bytes = read_buffer.read_fd(fd, nbytes);
  if (0 == read_bytes) {
    if (DSTORE_SUCCESS != (ret = on_close(connection_id))) {
      LOG_WARN("handle close failed, ret=%d", ret);
      return ret;
    }
    return ret;
  }
  if (DSTORE_SUCCESS != (ret = message_decoder_(conn))) {
    LOG_WARN("decode message failed, ret=%d", ret);
    return ret;
  }
  return new_message_callback_(conn);
}

int TCPClient::on_write(int fd, int type, void *args)
{
  int ret = DSTORE_SUCCESS;
  const int64_t connection_id = reinterpret_cast<int64_t>(args);
  std::shared_ptr<Connection> conn = connection_map_[connection_id];
  if (Connection::ACTIVE == conn->get_status()) {
    Buffer &write_buffer = conn->get_write_buffer();
    ssize_t write_bytes = write_buffer.write_fd(fd);
    if (-1 == write_bytes) {
      remove_connection(connection_id);
      return ret;
    }
    size_t need_write_bytes = write_buffer.get_need_write_bytes();
    if (0 == need_write_bytes) {
      if (DSTORE_SUCCESS != (ret = conn->remove_write())) {
        LOG_WARN("remove write event from connection_id=%d failed, ret=%d", connection_id, ret);
        return ret;
      }
      if (Connection::SHUTDOWN_READ == conn->get_status()) {
        if (DSTORE_SUCCESS != (ret = on_close(connection_id))) {
          LOG_WARN("handle close connection failed, ret=%d", ret);
          return ret;
        }
      }
    } else if (need_write_bytes > 0) {
      // do not modify the event
      // just let it in loop again
    }
  } else {
    bool ok = conn->is_connect_ok();
    if (ok) {
      conn->set_status(Connection::ACTIVE);
      if (DSTORE_SUCCESS != (ret = conn->remove_write())) {
        LOG_WARN("remove write event from connection=%d failed, ret=%d", connection_id, ret);
        return ret;
      }
      connect_complete_(conn);
    } else {
      remove_connection(connection_id);
      return ret;
    }
  }
  ret = DSTORE_CONNECT_IN_PROGRESS == ret ? DSTORE_SUCCESS : ret;
  return ret;
}

void TCPClient::remove_connection(const int64_t connection_id)
{
  std::shared_ptr<Connection> conn = connection_map_[connection_id];
  conn->close();
  connection_map_.erase(connection_id);
}

int TCPClient::on_close(const int64_t connection_id)
{
  int ret = DSTORE_SUCCESS;
  std::shared_ptr<Connection> conn = connection_map_[connection_id];
  assert(conn != nullptr);
  bool has_data_to_write = conn->pending_write();
  if (!has_data_to_write) {
    remove_connection(connection_id);
  } else {
    set_connection_status(connection_id, Connection::SHUTDOWN_READ);
  }
  return ret;
}

int TCPClient::set_connection_status(const int64_t connection_id, const Connection::Status status)
{
  int ret = DSTORE_SUCCESS;
  std::shared_ptr<Connection> conn = connection_map_[connection_id];
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
