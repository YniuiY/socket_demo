#ifndef UDP_SERVER_H_
#define UDP_SERVER_H_
/**
 * socket()
 * |
 * bind()
 * |
 * recvfrom() <-|
 * |            |
 * sendto()     |
 * |----------->|
 * close()
 */

#include "common/headers.h"

namespace udp {

class UpdServer {
 public:
  UpdServer();
  explicit UpdServer(int const& port);
  ~UpdServer();

  void Socket();
  void Bind();
  void Start();
 
 private:
  void run();
  void recv_data();
  void send_data();

  int server_socket_;
  sockaddr_in server_addr_;
  sockaddr_in client_addr_;
  int epoll_fd_;
  Packet* packet_;
  char buffer[MAX_UDP_DATA_SIZE];
};

} // namespace udp

#endif // UDP_SERVER_H_