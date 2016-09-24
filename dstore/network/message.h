#ifndef DSTORE_NETWORK_MESSAGE_H_
#define DSTORE_NETWORK_MESSAGE_H_

namespace dstore
{
namespace network
{
class Message
{
 public:
  explicit Message(void *request);
  void *get_request(void) { return request_; }
  void set_request(void *request) { request_ = request; }
 private:
  void *request_;
};
}  // end namespace network
}  // end namespace dstore

#endif  // DSTORE_NETWORK_MESSAGE_H_
