#ifndef DSTORE_COMMON_ERRNO_DEFINE_H_
#define DSTORE_COMMON_ERRNO_DEFINE_H_

namespace dstore
{
namespace common
{
#define DSTORE_SUCCESS 0
#define DSTORE_MEMORY_ALLOCATION_FAILED -1
#define DSTORE_EPOLL_ERROR -2
#define DSTORE_BIND_ERROR -3
#define DSTORE_LISTEN_ERROR -4
#define DSTORE_ACCEPT_ERROR -5
#define DSTORE_PARSE_ADDR_ERROR -6
#define DSTORE_PARSE_ADDR_EMPTY -7
#define DSTORE_SET_SOCKET_ERROR -8
#define DSTORE_EAGAIN -9
}  // end namespace common
}  // end namespace dstore

#endif  // DSTORE_COMMON_ERRNO_DEFINE_H_
