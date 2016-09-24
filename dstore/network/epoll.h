#ifndef DSTORE_NETWORK_EPOLL_H_
#define DSTORE_NETWORK_EPOLL_H_

#include <sys/epoll.h>
#include <memory>
#include "event_poll_api.h"

namespace dstore
{
namespace network
{
class EventLoop;
class EpollAPI : public EventPollAPI
{
 public:
  explicit EpollAPI(EventLoop *loop);
  virtual ~EpollAPI();
  virtual int init(void);
  virtual int register_event(int fd, int type);
  virtual int unregister_event(int fd, int type);
  virtual int modify_event(int fd, int type);
  virtual int resize(int set_size);
  virtual int loop(int timeout);
  virtual void destory(void);
  EpollAPI &operator=(const EpollAPI &) = delete;
  EpollAPI(const EpollAPI &) = delete;
 private:
  static const int LOOP_INIT_SIZE = 1024;
  int epfd_;
  struct epoll_event *events_;
  int loop_size_;
  std::shared_ptr<EventLoop> loop_;
};
}  // end namespace network
}  // end namespace dstore

#endif  // DSTORE_NETWORK_EPOLL_H_
