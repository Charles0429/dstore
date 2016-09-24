#include "connection.h"
#include "event_loop.h"
#include "errno_define.h"
#include "log.h"

using namespace dstore::network;

Connection::Connection(const Event &e, const InetAddr &peer)
  : status_(Connection::INVALID), e_(e), loop_(nullptr), socket_(e.fd),
  peer_(peer), message_list_(), read_buffer_(), write_buffer_()
{
}

int Connection::init(EventLoop &loop)
{
  int ret = DSTORE_SUCCESS;
  if (DSTORE_SUCCESS != (ret = socket_.set_nonblocking())) {
    LOG_INFO("set nonblocking failed, ret=%d", ret);
    return ret;
  }
  loop_ = &loop;
  return ret;
}

void Connection::set_event(const Event &e)
{
  e_ = e;
}

Event &Connection::get_event(void)
{
  return e_;
}

Socket &Connection::get_socket(void)
{
  return socket_;
}

Buffer &Connection::get_read_buffer(void)
{
  return read_buffer_;
}

Buffer &Connection::get_write_buffer(void)
{
  return write_buffer_;
}

int Connection::remove_write(void)
{
  int ret = DSTORE_SUCCESS;
  e_.type &= ~Event::kEventWrite;
  if (DSTORE_SUCCESS != (ret = loop_->modify_event(e_))) {
    LOG_INFO("modify event failed, fd=%d, type=%d, ret=%d", e_.fd, e_.type, ret);
    return ret;
  }
  return ret;
}

int Connection::add_write(void)
{
  int ret = DSTORE_SUCCESS;
  e_.type |= Event::kEventWrite;
  if (DSTORE_SUCCESS != (ret = loop_->modify_event(e_))) {
    LOG_INFO("modify event failed, fd=%d, type=%d, ret=%d", e_.fd, e_.type, ret);
    return ret;
  }
  return ret;
}

int Connection::remove_read(void)
{
  int ret = DSTORE_SUCCESS;
  e_.type &= ~Event::kEventRead;
  if (DSTORE_SUCCESS != (ret = loop_->modify_event(e_))) {
    LOG_INFO("modify event failed, fd=%d, type=%d, ret=%d", e_.fd, e_.type, ret);
    return ret;
  }
  return ret;
}

int Connection::add_read(void)
{
  int ret = DSTORE_SUCCESS;
  e_.type |= Event::kEventRead;
  if (DSTORE_SUCCESS != (ret = loop_->modify_event(e_))) {
    LOG_INFO("modify event failed, fd=%d, type=%d, ret=%d", e_.fd, e_.type, ret);
    return ret;
  }
  return ret;
}

void Connection::add_message(Message *message)
{
  message_list_.push_back(message);
}

void Connection::remove_message(Message *message)
{
  message_list_.remove(message);
}

std::list<Message *> &Connection::get_message_list(void)
{
  return message_list_;
}

void Connection::close(void)
{
  socket_.close();
}

void Connection::set_status(const Connection::Status status)
{
  status_ = status;
}

Connection::Status Connection::get_status(void)
{
  return status_;
}

bool Connection::pending_write(void)
{
  return write_buffer_.get_need_write_bytes() > 0;
}
