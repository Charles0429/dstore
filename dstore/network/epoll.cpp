#include "errno_define.h"
#include "log.h"
#include "memory.h"
#include "epoll.h"
#include "event_loop.h"

using namespace dstore::common;
using namespace dstore::network;

EpollAPI::EpollAPI(EventLoop *loop)
  : epfd_(0), events_(nullptr), loop_(loop)
{
}

EpollAPI::~EpollAPI()
{
  if (nullptr != events_) {
    dstore_free(events_);
  }
}

int EpollAPI::init(void)
{
  int ret = DSTORE_SUCCESS;
  events_ = static_cast<struct epoll_event *>(dstore_malloc(sizeof(struct epoll_event) * LOOP_INIT_SIZE));
  if (nullptr == events_) {
    ret = DSTORE_MEMORY_ALLOCATION_FAILED;
    LOG_ERROR("allocate memory failed for epoll events, ret=%d", ret);
    return ret;
  }
  loop_size_ = LOOP_INIT_SIZE;
  epfd_ = epoll_create(1024);
  if (-1 == epfd_) {
    ret = DSTORE_EPOLL_ERROR;
    LOG_ERROR("epoll_create failed, ret=%d", ret);
    dstore_free(events_);
    return ret;
  }

  return ret;
}

int EpollAPI::register_event(int fd, int type)
{
  struct epoll_event event;
  int sys_ret = 0;
  int ret = DSTORE_SUCCESS;

  event.data.fd = fd;
  event.events = 0;
  if (type & Event::Event::kEventRead) {
    event.events |= EPOLLIN;
  }
  if (type & Event::Event::kEventWrite) {
    event.events |= EPOLLOUT;
  }

  sys_ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &event);
  if (-1 == sys_ret) {
    ret = DSTORE_EPOLL_ERROR;
    LOG_WARN("epoll_ctl failed, epfd=%d, fd=%d, event=%d, errno=%d", epfd_, fd, event.events, errno);
  }
  return ret;
}

int EpollAPI::unregister_event(int fd, int type)
{
  struct epoll_event event;
  int sys_ret = 0;
  int ret = DSTORE_SUCCESS;

  event.events = 0;
  event.data.fd = fd;
  if (type & Event::kEventRead) {
    event.events |= EPOLLIN;
  }
  if (type & Event::kEventWrite) {
    event.events |= EPOLLOUT;
  }
  sys_ret = epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, &event);
  if (-1 == sys_ret) {
    ret = DSTORE_EPOLL_ERROR;
  }
  return ret;
}

int EpollAPI::modify_event(int fd, int type)
{
  struct epoll_event event;
  int sys_ret = 0;
  int ret = DSTORE_SUCCESS;

  event.events = 0;
  event.data.fd = 0;

  if (type & Event::kEventRead) {
    event.events |= EPOLLIN;
  }
  if (type & Event::kEventWrite) {
    event.events |= EPOLLOUT;
  }

  sys_ret = epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &event);
  if (-1 == sys_ret) {
    ret = DSTORE_EPOLL_ERROR;
  }
  return ret;
}

int EpollAPI::resize(int set_size)
{
  int ret = DSTORE_SUCCESS;
  struct epoll_event *events_ptr = nullptr;

  events_ptr = static_cast<struct epoll_event *>(dstore_realloc(events_, sizeof(struct epoll_event) * set_size));
  if (nullptr == events_ptr) {
    ret = DSTORE_MEMORY_ALLOCATION_FAILED;
    return ret;
  }
  events_ = events_ptr;
  loop_size_ = set_size;
  return ret;
}

int EpollAPI::loop(int timeout)
{
  int sys_ret = 0;
  int ret = DSTORE_SUCCESS;
  int events_num = 0;

  sys_ret = epoll_wait(epfd_, events_, loop_size_, timeout);
  if (-1 == sys_ret) {
    ret = DSTORE_EPOLL_ERROR;
    return ret;
  } else {
    events_num = sys_ret;
  }

  for (int64_t i = 0; i < events_num; ++i) {
    int event_type = 0;
    struct epoll_event *e = events_ + i;

    if (e->events & EPOLLERR) {
      event_type |= Event::kEventError;
    }
    if (e->events & (EPOLLIN | EPOLLHUP)) {
      event_type |= Event::kEventRead;
    }
    if (e->events & EPOLLOUT) {
      event_type |= Event::kEventWrite;
    }
    loop_->add_ready_event(e->data.fd, event_type);
  }
  if (events_num == loop_size_) {
    int tmp_ret = DSTORE_SUCCESS;
    if (DSTORE_SUCCESS != (tmp_ret = resize(loop_size_ * 2))) {
      LOG_WARN("epoll resize failed, loop_size_=%d, expand_to=%d, ret=%d", loop_size_, loop_size_ * 2, ret);
    }
  }
  return ret;
}

void EpollAPI::destory(void)
{
  if (nullptr != events_) {
    dstore_free(events_);
  }
}
