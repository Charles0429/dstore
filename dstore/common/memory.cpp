#include "memory.h"
#include <cstdlib>

namespace dstore
{
namespace common
{
void *dstore_malloc(size_t size)
{
  return ::malloc(size);
}

void dstore_free(void *ptr)
{
  return ::free(ptr);
}

void *dstore_realloc(void *ptr, size_t size)
{
  return ::realloc(ptr, size);
}

}  // end namespace common
}  // end namespace dstore
