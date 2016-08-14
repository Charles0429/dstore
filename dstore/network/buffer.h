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
  size_t get_write_bytes(void);
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
