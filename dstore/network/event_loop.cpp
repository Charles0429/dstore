#include "event_loop.h"
#include <cstddef>
#include "errno_define.h"
#include "log.h"
#include "event_poll_api.h"

using namespace dstore::common;
using namespace dstore::network;

EventLoop::EventLoop(void)
  : poll_api_(nullptr), registered_events_(),
  ready_events_(), timeout_(-1), stop_(false)
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

int EventLoop::register_event(const Event &e)
{
  int ret = DSTORE_SUCCESS;
  const int type = e.type;
  if ((type & Event::kEventRead) || (type & Event::kEventWrite)) {
    ret = poll_api_->register_event(e.fd, e.type);
    if (DSTORE_SUCCESS != ret) {
      LOG_WARN("register event failed, fd=%d, type=%d, ret=%d", e.fd, e.type, ret);
    } else {
      registered_events_[e.fd] = e;
    }
  }
  return ret;
}

int EventLoop::unregister_event(const Event &e)
{
  int ret = DSTORE_SUCCESS;
  const int type = e.type;
  if ((type & Event::kEventRead) || (type & Event::kEventWrite)) {
    ret = poll_api_->unregister_event(e.fd, e.type);
    if (DSTORE_SUCCESS != ret) {
      LOG_WARN("unregister event failed, fd=%d, type=%d, ret=%d", e.fd, e.type, ret);
    } else {
      registered_events_.erase(e.fd);
    }
  }
  return ret;
}

int EventLoop::modify_event(const Event &e)
{
  int ret = DSTORE_SUCCESS;
  const int type = e.type;
  if ((type & Event::kEventRead) || (type & Event::kEventWrite)) {
    ret = poll_api_->modify_event(e.fd, e.type);
    if (DSTORE_SUCCESS != ret) {
      LOG_WARN("modify event failed, ret=%d", ret);
    } else {
      registered_events_[e.fd] = e;
    }
  }
  return ret;
}

int EventLoop::loop(void)
{
  int ret = DSTORE_SUCCESS;
  while (!stop_) {
    if (DSTORE_SUCCESS != (ret = poll_api_->loop(timeout_))) {
      LOG_ERROR("poll error, ret=%d", ret);
      return ret;
    }
    for (auto e = ready_events_.cbegin(); e != ready_events_.cend(); ++e) {
      const Event &r = registered_events_[e->fd];
      if (e->type & r.type & Event::kEventRead) {
        e->read_cb(e->fd, Event::kEventRead, e->args);
      }
      if (e->type & r.type & Event::kEventWrite) {
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
  Event e = registered_events_[fd];
  e.type = event_type;
  ready_events_.push_back(e);
}

void EventLoop::clear_ready_events(void)
{
  ready_events_.clear();
}
