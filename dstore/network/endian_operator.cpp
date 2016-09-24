#include "endian_operator.h"
#include <endian.h>

namespace dstore
{
namespace network
{
uint16_t host_to_network16(uint16_t host)
{
  return htobe16(host);
}

uint16_t network_to_host16(uint16_t net)
{
  return be16toh(net);
}

uint32_t host_to_network32(uint32_t host)
{
  return htobe32(host);
}

uint32_t network_to_host32(uint32_t net)
{
  return be32toh(net);
}

uint64_t host_to_network64(uint64_t host)
{
  return htobe64(host);
}

uint64_t network_to_host64(uint64_t net)
{
  return be64toh(net);
}
}  // end namespace network
}  // end namespace dstore
