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

  void Socket();
  void Start();

 private:
  void send_pack();
  void recv_pack();

  sockaddr_in server_addr_;
  int socket_fd_;
  Packet* packet_;
  char buffer[MAX_UDP_DATA_SIZE];
};
} // namespace udp

#endif