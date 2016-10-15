#ifndef DSTORE_NETWORK_EVENT_LOOP_H_
#define DSTORE_NETWORK_EVENT_LOOP_H_

#include <unordered_map>
#include <list>
#include <functional>
#include <memory>
#include <vector>

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
  static const int kEventTimer = 1 << 2;
  static const int kEventError = 1 << 3;
  int fd;
  int type;
  void *args;
  int64_t timeout;
  std::vector<Event *>::size_type index;
  EventCallback read_cb;
  EventCallback write_cb;
  EventCallback timer_cb;
};

class EventLoop
{
 public:
  EventLoop(void);
  ~EventLoop(void);
  int init(EventPollAPI *api);
  int register_event(Event *e);
  int unregister_event(Event *e);
  int modify_event(Event *e);
  int loop(void);
  void destroy(void);
  int get_loop_size(void);
  void add_ready_event(int fd, int event_type);
  EventLoop &operator=(const EventLoop &) = delete;
  EventLoop(const EventLoop &) = delete;
 private:
  typedef std::vector<Event *>::size_type Index;
  void clear_ready_events(void);
  int register_timer_event(Event *e);
  int unregister_timer_event(const Event *e);
  int modify_timer_event(Event *e);
  void swap_timer_event(const Index i, const Index j);
  void down_timer_heap(const Index i, const Index size, std::vector<Event *> &heap);
  void up_timer_heap(const Index i, std::vector<Event *> &heap);
  void adjust_timer_heap(const Index i, const Index size, std::vector<Event *> &heap);
  void push_timer_heap(Event *e);
  void pop_timer_heap(void);
  Event *top_timer_heap(void);
  void remove_timer_heap(const Event *e);
  void update_timer_heap(const Event *e);
  int get_timeout(void);
  void process_timeout_events(void);
 private:
  std::unique_ptr<EventPollAPI> poll_api_;
  std::unordered_map<int, Event *> registered_events_;
  std::list<Event> ready_events_;
  std::vector<Event *> timer_heap_;
  bool stop_;
};
}  // end namespace network
}  // end namespace dstore

#endif  // DSTORE_NETWORK_EVENT_LOOP_H_
