#include <list>
#include <iostream>
#include <memory>
#include "tcp_server.h"
#include "connection.h"
#include "message.h"
#include "errno_define.h"
#include "log.h"

using namespace dstore::common;
using namespace dstore::network;

struct simple_packet
{
  int32_t data_len;
  char *data;
};

int decode_message(std::shared_ptr<Connection> conn)
{
  int ret = DSTORE_SUCCESS;
  const size_t header_len = sizeof(int32_t);
  Buffer &read_buffer = conn->get_read_buffer();
  size_t readable_bytes = read_buffer.get_read_bytes();
  if (readable_bytes < header_len) {
    ret = DSTORE_EAGAIN;
    return ret;
  }
  LOG_INFO("header enough, readable_bytes = %d", readable_bytes);
  char *data = read_buffer.get_data();
  int32_t data_len = read_buffer.peek_int32();
  LOG_INFO("data len=%d\n", data_len);
  if (readable_bytes < data_len + header_len) {
    ret = DSTORE_EAGAIN;
    return ret;
  }
  simple_packet *packet = new simple_packet();
  packet->data_len = data_len;
  packet->data = data + header_len;
  Message *message = new Message(static_cast<void *>(packet));
  conn->add_message(message);
  read_buffer.consume(data_len + header_len);
  return ret;
}

int process_message(std::shared_ptr<Connection> conn)
{
  int ret = DSTORE_SUCCESS;
  LOG_INFO("start processing message");
  std::list<Message *> &message_list = conn->get_message_list();
  for (auto iter = message_list.begin(); iter != message_list.end();) {
    LOG_INFO("start dealing with message");
    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "hello world");
    Buffer &write_buffer = conn->get_write_buffer();
    write_buffer.append_int32(static_cast<int32_t>(strlen(buffer)));
    write_buffer.append(buffer, strlen(buffer));
    LOG_INFO("writetable_bytes=%d", write_buffer.get_need_write_bytes());
    if (DSTORE_SUCCESS != (ret = conn->add_write())) {
      LOG_INFO("add write event failed, ret=%d", ret);
      return ret;
    }
    simple_packet *packet = static_cast<simple_packet *>((*iter)->get_request());
    delete packet;
    delete *iter;
    iter = message_list.erase(iter);
  }
  return ret;
}

int main(int argc, char **argv)
{
  TCPServer server("0.0.0.0", "8900", false);
  server.set_message_decode_callback(decode_message);
  server.set_new_message_callback(process_message);
  int ret = DSTORE_SUCCESS;
  if (DSTORE_SUCCESS != (ret = server.start())) {
    LOG_ERROR("can not start server, ret=%d", ret);
    return ret;
  }
  if (DSTORE_SUCCESS != (ret = server.loop())) {
    LOG_ERROR("loop failed, ret=%d", ret);
    return ret;
  }
  return ret;
}
