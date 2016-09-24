#ifndef DSTORE_NETWORK_EVENT_LOOP_H_
#define DSTORE_NETWORK_EVENT_LOOP_H_

#include <unordered_map>
#include <list>
#include <functional>
#include <memory>

namespace dstore
{
namespace network
{
class EventPollAPI;

struct Event
{
 public:
  typedef std::function<int(int, int, void *)> EventCallback;
  static const int kEventRead = 1 << 0;
  static const int kEventWrite = 1 << 1;
  static const int kEventError = 1 << 2;
  int fd;
  int type;
  void *args;
  EventCallback read_cb;
  EventCallback write_cb;
};

class EventLoop
{
 public:
  EventLoop(void);
  ~EventLoop(void);
  int init(EventPollAPI *api);
  int register_event(const Event &e);
  int unregister_event(const Event &e);
  int modify_event(const Event &e);
  int loop(void);
  void destroy(void);
  int get_loop_size(void);
  void add_ready_event(int fd, int event_type);
  EventLoop &operator=(const EventLoop &) = delete;
  EventLoop(const EventLoop &) = delete;
 private:
  void clear_ready_events(void);
 private:
  std::unique_ptr<EventPollAPI> poll_api_;
  std::unordered_map<int, Event> registered_events_;
  std::list<Event> ready_events_;
  int timeout_;
  bool stop_;
};
}  // end namespace network
}  // end namespace dstore

#endif  // DSTORE_NETWORK_EVENT_LOOP_H_
