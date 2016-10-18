#include <vector>
#include "event_loop.h"
#include "epoll.h"
#include "time_operator.h"
#include "log.h"
#include "errno_define.h"

using namespace dstore::common;
using namespace dstore::network;
using namespace std::placeholders;

int timer_func(int fd, int type, void *args)
{
  int ret = DSTORE_SUCCESS;
  LOG_INFO("time %d: now %ld", fd, get_milliseconds());
  return ret;
}

int main(void)
{
  int ret = DSTORE_SUCCESS;
  EventLoop loop;
  EventPollAPI *poll_api = new EpollAPI(&loop);
  if (DSTORE_SUCCESS != (ret = loop.init(poll_api))) {
    LOG_WARN("init event loop failed");
    return ret;
  }

  std::vector<Event *> events;
  for (int i = 0; i < 100; i++) {
    Event *e = new Event();
    e->type = Event::kEventTimer;
    e->timeout = 1000 * (i+1);
    e->timer_cb = std::bind(timer_func, _1, _2, _3);
    e->fd = i;
    LOG_INFO("try registering timer=%ld, timeout=%ld", i, e->timeout);
    if (DSTORE_SUCCESS != (ret = loop.register_event(e))) {
      LOG_WARN("register event failed, i = %d", i);
      return ret;
    }
    events.push_back(e);
  }
  for (int i = 10; i < 100; i += 10) {
    LOG_INFO("modify timer to 10S");
    events[i]->timeout = 100 * 1000;
    if (DSTORE_SUCCESS != (ret = loop.modify_event(events[i]))) {
      LOG_WARN("modify event failed, ret=%d", ret);
      return ret;
    }
  }
  for (int i = 9; i < 100; i += 9) {
    LOG_INFO("unregistering timers");
    Event *e = events[i];
    if (DSTORE_SUCCESS != (ret = loop.unregister_event(e))) {
      LOG_WARN("unregister event failed");
      return ret;
    }
  }
  if (DSTORE_SUCCESS != (ret = loop.loop())) {
    LOG_WARN("loop failed, ret=%d", ret);
    return ret;
  }
  return ret;
}
