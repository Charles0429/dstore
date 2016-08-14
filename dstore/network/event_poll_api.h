#ifndef DSTORE_NETWORK_EVENT_POLL_API_H_
#define DSTORE_NETWORK_EVENT_POLL_API_H_

#include <cstdint>

namespace dstore
{
namespace network
{
class EventPollAPI
{
 public:
  EventPollAPI() {}
  virtual ~EventPollAPI() {}
  virtual int init(void) = 0;
  virtual int register_event(int fd, int type) = 0;
  virtual int unregister_event(int fd, int type) = 0;
  virtual int modify_event(int fd, int type) = 0;
  virtual int resize(int set_size) = 0;
  virtual int loop(int timeout) = 0;
  virtual void destory(void) = 0;
  EventPollAPI &operator=(const EventPollAPI &) = delete;
  EventPollAPI(const EventPollAPI &) = delete;
};
}  // end namespace network
}  // end namespace dstore

#endif  // DSTORE_NETWORK_EVENT_POLL_API_H_
