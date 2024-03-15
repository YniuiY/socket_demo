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
  UpdServer(int const& port, std::string const& ip);
  ~UpdServer();

  void Socket(bool is_multi_cast);
  void Bind();
  void Start();
 
 private:
  void run();
  void recv_data();
  void send_data();

  int server_socket_;
  std::string multi_cast_ip_;
  sockaddr_in server_addr_;
  sockaddr_in client_addr_;
  uint8_t recv_buffer[MAX_UDP_DATA_SIZE];
  uint8_t send_buffer[MAX_UDP_DATA_SIZE];
};

} // namespace udp

#endif // UDP_SERVER_H_