#include "event_loop.h"
#include <cstddef>
#include <limits>
#include <utility>
#include "errno_define.h"
#include "log.h"
#include "event_poll_api.h"
#include "time_operator.h"

using namespace dstore::common;
using namespace dstore::network;

EventLoop::EventLoop(void)
  : poll_api_(nullptr), registered_events_(),
  ready_events_(), timer_heap_(), stop_(false)
{
}

EventLoop::~EventLoop(void)
{
  destroy();
}

int EventLoop::init(EventPollAPI *poll_api)
{
  int ret = DSTORE_SUCCESS;
  if (DSTORE_SUCCESS != (ret = poll_api->init())) {
    LOG_ERROR("init poll api failed, ret=%d", ret);
    return ret;
  }
  poll_api_.reset(poll_api);
  return ret;
}

int EventLoop::register_event(Event *e)
{
  int ret = DSTORE_SUCCESS;
  const int type = e->type;
  if ((type & Event::kEventRead) || (type & Event::kEventWrite)) {
    if (DSTORE_SUCCESS != (ret = poll_api_->register_event(e->fd, e->type))) {
      LOG_WARN("register event failed, fd=%d, type=%d, ret=%d", e->fd, e->type, ret);
      return ret;
    } else {
      registered_events_[e->fd] = e;
    }
  }
  if (type & Event::kEventTimer) {
    if (DSTORE_SUCCESS != (ret = register_timer_event(e))) {
      LOG_WARN("register timer event failed");
      return ret;
    }
  }
  return ret;
}

int EventLoop::unregister_event(Event *e)
{
  int ret = DSTORE_SUCCESS;
  const int type = e->type;
  if ((type & Event::kEventRead) || (type & Event::kEventWrite)) {
    ret = poll_api_->unregister_event(e->fd, e->type);
    if (DSTORE_SUCCESS != ret) {
      LOG_WARN("unregister event failed, fd=%d, type=%d, ret=%d", e->fd, e->type, ret);
    } else {
      registered_events_.erase(e->fd);
    }
  }
  if (type & Event::kEventTimer) {
    if (DSTORE_SUCCESS != (ret = unregister_timer_event(e))) {
      LOG_WARN("unregister timer event failed");
      return ret;
    }
  }
  return ret;
}

int EventLoop::modify_event(Event *e)
{
  int ret = DSTORE_SUCCESS;
  const int type = e->type;
  if ((type & Event::kEventRead) || (type & Event::kEventWrite)) {
    ret = poll_api_->modify_event(e->fd, e->type);
    if (DSTORE_SUCCESS != ret) {
      LOG_WARN("modify event failed, ret=%d", ret);
    } else {
      registered_events_[e->fd] = e;
    }
  }
  if (type & Event::kEventTimer) {
    e->timeout += get_milliseconds();
    if (DSTORE_SUCCESS != (ret = modify_timer_event(e))) {
      LOG_WARN("modify timer event failed");
      return ret;
    }
  }
  return ret;
}

int EventLoop::loop(void)
{
  int ret = DSTORE_SUCCESS;
  while (!stop_) {
    if (DSTORE_SUCCESS != (ret = poll_api_->loop(get_timeout()))) {
      LOG_ERROR("poll error, ret=%d", ret);
      return ret;
    }
    process_timeout_events();
    for (auto e = ready_events_.cbegin(); e != ready_events_.cend(); ++e) {
      const Event *r = registered_events_[e->fd];
      if (e->type & r->type & Event::kEventRead) {
        e->read_cb(e->fd, Event::kEventRead, e->args);
      }
      if (e->type & r->type & Event::kEventWrite) {
        e->write_cb(e->fd, Event::kEventWrite, e->args);
      }
    }
    clear_ready_events();
  }
  return ret;
}

void EventLoop::destroy(void)
{
  poll_api_->destory();
}

void EventLoop::add_ready_event(int fd, int event_type)
{
  Event e = *registered_events_[fd];
  e.type = event_type;
  ready_events_.push_back(e);
}

void EventLoop::clear_ready_events(void)
{
  ready_events_.clear();
}

int EventLoop::register_timer_event(Event *e)
{
  int ret = DSTORE_SUCCESS;
  if ((e->timeout <= 0) || (e->timeout > std::numeric_limits<int32_t>::max())) {
    ret = DSTORE_INVALID_ARGUMENT;
    LOG_WARN("Invalid timeout=%ld", e->timeout);
    return ret;
  }
  push_timer_heap(e);
  return ret;
}

int EventLoop::unregister_timer_event(const Event *e)
{
  int ret = DSTORE_SUCCESS;
  if (e->index >= timer_heap_.size()) {
    ret = DSTORE_INVALID_ARGUMENT;
    LOG_WARN("Invalid event index=%d", e->index);
    return ret;
  }
  remove_timer_heap(e);
  return ret;
}

int EventLoop::modify_timer_event(Event *e)
{
  int ret = DSTORE_SUCCESS;
  if (e->index >= timer_heap_.size()) {
    ret = DSTORE_INVALID_ARGUMENT;
    LOG_WARN("Invalid event index=%d", e->index);
    return ret;
  }
  update_timer_heap(e);
  return ret;
}

void EventLoop::swap_timer_event(const Index i, const Index j)
{
  std::swap(timer_heap_[i], timer_heap_[j]);
  std::swap(timer_heap_[i]->index, timer_heap_[j]->index);
}

void EventLoop::down_timer_heap(const Index i, const Index size, std::vector<Event *> &heap)
{
  Index p = i;
  for (Index child = 2 * p + 1; child < size; child = 2 * p + 1) {
    if (child + 1 < size && heap[child]->timeout > heap[child+1]->timeout) {
      ++child;
    }
    if (heap[p]->timeout > heap[child]->timeout) {
      swap_timer_event(p, child);
      p = child;
    } else {
      break;
    }
  }
}

void EventLoop::up_timer_heap(const Index i, std::vector<Event *> &heap)
{
  int64_t c = static_cast<int64_t>(i);
  if (0 == i) {
    return;
  }
  for (int64_t parent = (c - 1) / 2; parent >= 0; parent = (parent - 1) / 2) {
    if (parent == c || heap[parent]->timeout < heap[c]->timeout) {
      break;
    } else {
      swap_timer_event(parent, c);
      c = parent;
    }
  }
}

void EventLoop::adjust_timer_heap(const Index i, const Index size, std::vector<Event *> &heap)
{
  if (i > 0 && heap[i]->timeout <= heap[(i-1)/2]->timeout) {
    up_timer_heap(i, heap);
  } else {
    down_timer_heap(i, size, heap);
  }
}

void EventLoop::push_timer_heap(Event *e)
{
  e->index = timer_heap_.size();
  e->timeout += get_milliseconds();
  timer_heap_.push_back(e);
  adjust_timer_heap(e->index, timer_heap_.size(), timer_heap_);
}

void EventLoop::pop_timer_heap(void)
{
  swap_timer_event(0, timer_heap_.size() - 1);
  adjust_timer_heap(0, timer_heap_.size() - 1, timer_heap_);
  timer_heap_.pop_back();
}

Event *EventLoop::top_timer_heap(void)
{
  return timer_heap_.size() == 0 ? nullptr : timer_heap_.front();
}

void EventLoop::remove_timer_heap(const Event *e)
{
  timer_heap_[e->index]->timeout = -1;
  adjust_timer_heap(e->index, timer_heap_.size(), timer_heap_);
  pop_timer_heap();
}

void EventLoop::update_timer_heap(const Event *e)
{
  adjust_timer_heap(e->index, timer_heap_.size(), timer_heap_);
}

int EventLoop::get_timeout(void)
{
  const int64_t now = get_milliseconds();
  return timer_heap_.size() == 0 ? -1 : static_cast<int>(timer_heap_.front()->timeout - now);
}

void EventLoop::process_timeout_events(void)
{
  const int64_t now = get_milliseconds();
  Event *e = top_timer_heap();
  while (e != nullptr && e->timeout <= now) {
    e->timer_cb(e->fd, Event::kEventTimer, e->args);
    pop_timer_heap();
    e = top_timer_heap();
  }
}
