#include "connection.h"
#include "event_loop.h"

using namespace dstore::network;

Connection::Connection(int fd, const InetAddr &peer)
  : e_(), socket_(fd), peer_(peer), message_list_(),
  read_buffer_(), write_buffer_()
{
}

void Connection::set_event(const Event &e)
{
  e_ = e;
}

Event &Connection::get_event(void)
{
  return e_;
}

Buffer &Connection::get_read_buffer(void)
{
  return read_buffer_;
}

Buffer &Connection::get_write_buffer(void)
{
  return write_buffer_;
}

void Connection::remove_write(void)
{
  e_.type &= ~Event::kEventWrite;
}

void Connection::add_write(void)
{
  e_.type |= Event::kEventWrite;
}

void Connection::remove_read(void)
{
  e_.type &= ~Event::kEventRead;
}

void Connection::add_read(void)
{
  e_.type |= Event::kEventRead;
}
