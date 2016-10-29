set(NETWORK_SRCS
  endian_operator.cpp
  buffer.cpp
  connection.cpp
  epoll.cpp
  event_loop.cpp
  inet_addr.cpp
  message.cpp
  socket.cpp
  socket_operator.cpp
  tcp_listener.cpp
  tcp_server.cpp
  tcp_connector.cpp
  tcp_client.cpp
  PARENT_SCOPE
)
MESSAGE(STATUS "${NETWORK_SRCS}")
