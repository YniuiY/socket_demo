#include "udp/udp_client.h"

namespace udp {

UdpClient::UdpClient():packet_{nullptr}, socket_fd_{-1} {
  std::string server_ip{"10.1.2.147"};
  memset(&server_addr_, 0, sizeof(server_addr_));
  server_addr_.sin_family = AF_INET;
  server_addr_.sin_port = htons(51016);
  inet_pton(AF_INET, server_ip.c_str(), &server_addr_.sin_addr);
}

UdpClient::UdpClient(std::string const& server_ip, int const& port):packet_{nullptr}, socket_fd_{-1} {
  memset(&server_addr_, 0, sizeof(server_addr_));
  server_addr_.sin_family = AF_INET;
  server_addr_.sin_port = htons(port);
  inet_pton(AF_INET, server_ip.c_str(), &server_addr_.sin_addr);  
}

UdpClient::~UdpClient() {
  close(socket_fd_);
  free(packet_);
  packet_ = nullptr;
}

void UdpClient::Socket() {
  socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_fd_ < 0) {
    std::cerr << "Create socket failed, " << strerror(errno) << std::endl;
    exit(1);
  }
  std::cout << "Creaet Socket Success\n";
}

void UdpClient::Start() {
  // todo: 1. 发hello； 2.接收回复； 3.关闭
  if (packet_ == nullptr) {
    packet_ = new Packet();
    memset(packet_, 0, sizeof(Packet));
  }

  int pack_header_size{sizeof(Packet::header)};
  int total_pack_size{0};

  std::string hello_msg{"Hello Server"};
  packet_ = (Packet*)malloc(hello_msg.size());
  packet_->header.data_size = hello_msg.size();
  strncpy(packet_->data, hello_msg.c_str(), hello_msg.size());

  total_pack_size = pack_header_size + packet_->header.data_size;
  int send_data_size = sendto(socket_fd_, packet_, total_pack_size, 0,
                              (sockaddr*)&server_addr_, sizeof(server_addr_));
  if (send_data_size != total_pack_size) {
    std::cerr << "Send pack data size error, real send size: " << send_data_size
              << ", need send size: " << total_pack_size << std::endl;
    exit(1);
  }
  std::cout << "Send request msg: " << hello_msg << " to server, send data size: " << send_data_size << std::endl;

  memset(buffer, 0, sizeof(buffer));
  socklen_t addr_len = sizeof(server_addr_);
  int recv_data_size = recvfrom(socket_fd_, buffer, MAX_UDP_DATA_SIZE, 0,
                                  (sockaddr*)&server_addr_, &addr_len);
  if (recv_data_size < 0) {
    std::cerr << "Recv data failed, " << strerror(errno) << std::endl;
    exit(1);
  } else {
    uint32_t data_size;
    memcpy(&data_size, buffer, sizeof(Packet::header));
    std::cout << "Need recv data size: " << data_size
              << ", real recv size: " << recv_data_size - sizeof(Packet::header)
              << std::endl;
    std::string recv_msg_str{buffer + sizeof(Packet::header)};
    std::cout << "Recv server response: " << recv_msg_str << std::endl;
  }
}

}