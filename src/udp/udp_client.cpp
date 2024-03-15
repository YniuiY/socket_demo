#include "udp/udp_client.h"

namespace udp {

UdpClient::UdpClient():socket_fd_{-1} {
  std::string server_ip{"10.1.2.147"};
  memset(&server_addr_, 0, sizeof(server_addr_));
  server_addr_.sin_family = AF_INET;
  server_addr_.sin_port = htons(51016);
  inet_pton(AF_INET, server_ip.c_str(), &server_addr_.sin_addr);
}

UdpClient::UdpClient(std::string const& server_ip, int const& port):socket_fd_{-1} {
  memset(&server_addr_, 0, sizeof(server_addr_));
  server_addr_.sin_family = AF_INET;
  server_addr_.sin_port = htons(port);
  inet_pton(AF_INET, server_ip.c_str(), &server_addr_.sin_addr);
}

UdpClient::~UdpClient() {
  close(socket_fd_);
}

void UdpClient::Socket(bool is_broadcast) {
  socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_fd_ < 0) {
    std::cerr << "Create socket failed, " << strerror(errno) << std::endl;
    exit(1);
  }
  std::cout << "Creaet Socket Success\n";
  if (is_broadcast) {
    const int on = 1;
    if (setsockopt(socket_fd_, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on)) < 0) {
      std::cerr << "setsockopt to broadcast failed, " << strerror(errno) << std::endl;
      exit(1);
    }
    std::cout << "Set socket to broadcast\n";
  }
}

void UdpClient::Start() {
  // todo: 1. 发hello； 2.接收回复； 3.关闭
  memset(send_buffer, 0, MAX_UDP_DATA_SIZE);
  memset(recv_buffer, 0, MAX_UDP_DATA_SIZE);
  Packet* pack = reinterpret_cast<Packet*>(send_buffer);
  int pack_header_size{sizeof(Packet::header)};
  int total_pack_size{0};

  std::string hello_msg{"Hello Server"};
  pack->header.data_size = hello_msg.size();
  strncpy(pack->data, hello_msg.c_str(), hello_msg.size());

  total_pack_size = pack_header_size + pack->header.data_size;
  int send_data_size = sendto(socket_fd_, pack, total_pack_size, 0,
                              (sockaddr*)&server_addr_, sizeof(server_addr_));
  if (send_data_size != total_pack_size) {
    std::cerr << "Send pack data size error, " << strerror(errno) << std::endl;
    exit(1);
  }
  std::cout << "Send request msg: " << hello_msg << " to server, send data size: " << send_data_size << std::endl;


  socklen_t addr_len = sizeof(server_addr_);
  pack = reinterpret_cast<Packet*>(recv_buffer);
  int recv_data_size = recvfrom(socket_fd_, pack, MAX_UDP_DATA_SIZE, 0,
                                  (sockaddr*)&server_addr_, &addr_len);
  if (recv_data_size < 0) {
    std::cerr << "Recv data failed, " << strerror(errno) << std::endl;
    exit(1);
  } else {
    uint32_t data_size = pack->header.data_size + pack_header_size;
    std::cout << "Need recv data size: " << data_size
              << ", real recv size: " << recv_data_size
              << std::endl;
    std::string recv_msg_str{pack->data};
    std::cout << "Recv server response: " << recv_msg_str << std::endl;
  }
}

}