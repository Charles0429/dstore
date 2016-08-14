#ifndef DSTORE_COMMON_MEMORY_H_
#define DSTORE_COMMON_MEMORY_H_

#include <cstddef>

namespace dstore
{
namespace common
{
void *dstore_malloc(size_t size);
void dstore_free(void *ptr);
void *dstore_realloc(void *ptr, size_t size);
}  // end namespace common
}  // end namespace dstore
#endif  // DSTORE_COMMON_MEMORY_H_
