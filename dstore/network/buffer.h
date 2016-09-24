#ifndef DSTORE_NETWORK_BUFFER_H_
#define DSTORE_NETWORK_BUFFER_H_

#include <sys/types.h>
#include <vector>

namespace dstore
{
namespace network
{
class Buffer
{
 public:
  Buffer(void);
  explicit Buffer(int size);
  ssize_t read_fd(int fd, size_t nbytes);
  ssize_t write_fd(int fd);
  void append(const char *buf, size_t nbytes);
  void append(const void *buf, size_t nbytes);
  void append_int16(int16_t data);
  void append_int32(int32_t data);
  void append_int64(int64_t data);
  int16_t get_int16(void);
  int32_t get_int32(void);
  int64_t get_int64(void);
  int16_t peek_int16(void);
  int32_t peek_int32(void);
  int64_t peek_int64(void);
  void *peek(void);
  void forward(size_t nbytes);
  size_t get_read_bytes(void);
  size_t get_need_write_bytes(void);
  size_t get_read_pos(void);
  size_t get_write_pos(void);
  char *get_data(void);
  void set_read_pos(size_t read_pos);
  void set_write_pos(size_t write_pos);
  Buffer &operator=(const Buffer &) = delete;
  Buffer(const Buffer &) = delete;
 private:
  size_t left(void);
  void resize(size_t nbytes);
 private:
  std::vector<char> buffer_;
  size_t read_pos_;
  size_t write_pos_;
};
}  // end namespace network
}  // end namespace dstore

#endif  // DSTORE_NETWORK_BUFFER_H_
