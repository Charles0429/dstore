#ifndef DSTORE_NETWORK_ENDIAN_H_
#define DSTORE_NETWORK_ENDIAN_H_

#include <cstdint>

namespace dstore
{
namespace network
{
  uint16_t network_to_host16(uint16_t network);
  uint16_t host_to_network16(uint16_t host);
  uint16_t network_to_host16(uint16_t net);
  uint32_t host_to_network32(uint32_t host);
  uint32_t network_to_host32(uint32_t net);
  uint64_t host_to_network64(uint64_t host);
  uint64_t network_to_host64(uint64_t net);
}  // end namespace network
}  // end namespace dstore

#endif  // DSTORE_NETWORK_ENDIAN_H_
