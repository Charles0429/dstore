#include "message.h"

using namespace dstore::network;

Message::Message(void *request)
  : request_(request)
{
}
