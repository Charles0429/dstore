#include "buffer.h"
#include "socket_operator.h"

using namespace dstore::network;

Buffer::Buffer(void)
  : buffer_(), read_pos_(0), write_pos_(0)
{
}

Buffer::Buffer(int size)
  : buffer_(), read_pos_(0), write_pos_(0)
{
  buffer_.reserve(size);
}

ssize_t Buffer::read_fd(int fd, size_t nbytes)
{
  ssize_t bytes_read = 0;
  resize(nbytes);
  bytes_read = recv(fd, buffer_.data() + write_pos_, nbytes);
  write_pos_ += (bytes_read > 0) ? bytes_read : 0;
  return bytes_read;
}

ssize_t Buffer::write_fd(int fd)
{
  ssize_t bytes_writen = 0;
  bytes_writen = send(fd, buffer_.data() + read_pos_, get_write_bytes());
  read_pos_ += (bytes_writen > 0) ? bytes_writen : 0;
  return bytes_writen;
}

size_t Buffer::get_write_bytes(void)
{
  return write_pos_ - read_pos_;
}

size_t Buffer::left(void)
{
  return buffer_.capacity() - write_pos_;
}

void Buffer::resize(size_t nbytes)
{
  size_t left_bytes = left();
  if (nbytes > left_bytes) {
    size_t expand = buffer_.capacity() + 2 * (nbytes - left_bytes);
    buffer_.resize(expand);
  }
}
