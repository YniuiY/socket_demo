#ifndef UDP_CLIENT_H_
#define UDP_CLIENT_H_

/**
 * socket()
 * |
 * sendto() <---|
 * |            |
 * recvform() ->|
 * |
 * close()
 */

#include "common/headers.h"

namespace udp {

class UdpClient {
 public:
  UdpClient();
  UdpClient(std::string const& server_ip, int const& port);
  ~UdpClient();

  void Socket(bool is_broadcast);
  void Start();

 private:
  void send_pack();
  void recv_pack();

  sockaddr_in server_addr_;
  int socket_fd_;
  uint8_t recv_buffer[MAX_UDP_DATA_SIZE];
  uint8_t send_buffer[MAX_UDP_DATA_SIZE];
};
} // namespace udp

#endif