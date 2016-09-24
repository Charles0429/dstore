#include "buffer.h"
#include <cstring>
#include <cassert>
#include "socket_operator.h"
#include "endian_operator.h"

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
  bytes_writen = send(fd, buffer_.data() + read_pos_, get_need_write_bytes());
  read_pos_ += (bytes_writen > 0) ? bytes_writen : 0;
  return bytes_writen;
}

void Buffer::append(const char *buf, size_t nbytes)
{
  resize(nbytes);
  std::copy(buf, buf + nbytes, buffer_.data() + write_pos_);
  write_pos_ += nbytes;
}

void Buffer::append(const void *buf, size_t nbytes)
{
  return append(static_cast<const char *>(buf), nbytes);
}

void Buffer::append_int16(int16_t data)
{
  int16_t network = dstore::network::host_to_network16(data);
  append(&network, sizeof(int16_t));
}

void Buffer::append_int32(int32_t data)
{
  int32_t network = host_to_network32(data);
  append(&network, sizeof(int32_t));
}

void Buffer::append_int64(int64_t data)
{
  int64_t network = host_to_network64(data);
  append(&network, sizeof(int64_t));
}

void *Buffer::peek(void)
{
  return buffer_.data();
}

void Buffer::forward(size_t nbytes)
{
  read_pos_ += nbytes;
}

int16_t Buffer::peek_int16(void)
{
  int16_t ret = -1;
  ::memcpy(&ret, peek(), sizeof(ret));
  return network_to_host16(ret);
}

int16_t Buffer::get_int16(void)
{
  assert(get_read_bytes() >= sizeof(int16_t));
  int16_t ret = peek_int16();
  forward(sizeof(int16_t));
  return ret;
}

int32_t Buffer::peek_int32(void)
{
  int32_t ret = -1;
  ::memcpy(&ret, peek(), sizeof(ret));
  return network_to_host32(ret);
}

int32_t Buffer::get_int32(void)
{
  assert(get_read_bytes() >= sizeof(int32_t));
  int32_t ret = peek_int32();
  forward(sizeof(int32_t));
  return ret;
}

int64_t Buffer::peek_int64(void)
{
  int64_t ret = -1;
  ::memcpy(&ret, peek(), sizeof(ret));
  return network_to_host64(ret);
}

int64_t Buffer::get_int64(void)
{
  assert(get_read_bytes() >= sizeof(int64_t));
  int64_t ret = peek_int64();
  forward(sizeof(int64_t));
  return ret;
}

size_t Buffer::get_read_pos(void)
{
  return read_pos_;
}

size_t Buffer::get_write_pos(void)
{
  return write_pos_;
}

void Buffer::set_read_pos(size_t read_pos)
{
  read_pos_ = read_pos;
}

void Buffer::set_write_pos(size_t write_pos)
{
  write_pos_ = write_pos;
}

char *Buffer::get_data(void)
{
  return buffer_.data();
}

size_t Buffer::get_read_bytes(void)
{
  return write_pos_ - read_pos_;
}

size_t Buffer::get_need_write_bytes(void)
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
