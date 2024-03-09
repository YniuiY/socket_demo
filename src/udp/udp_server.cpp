/**
 * 通常情况下tcp服务器是并发的，UDP服务器是迭代的
 * 所以一般来说UDP服务器不需要使用epoll这类I/O多路复用的方法。
 * 具体来看就是，UDP接收，发送数据始终用的同一个socket描述符，
 * 不存在TCP在accept后返回新链接socket描述符，使用新描述符通信的机制。
 * 
 * 注：本例程使用epoll纯属练习epoll使用方法用，并不是要多路复用
 */

#include "udp/udp_server.h"

namespace udp {

UpdServer::UpdServer():server_socket_{-1}, epoll_fd_{-1}, packet_{nullptr} {
  memset(&server_addr_, 0, sizeof(server_addr_));
  server_addr_.sin_family = AF_INET;
  server_addr_.sin_port = htons(51016);
  server_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
}

UpdServer::UpdServer(int const& port):server_socket_{-1}, epoll_fd_{-1}, packet_{nullptr} {
  memset(&server_addr_, 0, sizeof(server_addr_));
  server_addr_.sin_family = AF_INET;
  server_addr_.sin_port = htons(port);
  server_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
}

UpdServer::~UpdServer() {
  close(server_socket_);
  close(epoll_fd_);
  free(packet_);
  packet_ = nullptr;
}

void UpdServer::Socket() {
  server_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
  if (server_socket_ == -1) {
    std::cerr << "Create udp socket failed\n";
    exit(1);
  } else {
    std::cout << "Create udp server socket success\n";
  }
}

void UpdServer::Bind() {
  bind(server_socket_, (sockaddr*)&server_addr_, sizeof(server_addr_));
  std::cout << "Upd server bind\n";
}

void UpdServer::Start() {
  std::cout << "Udp server start\n";
  epoll_fd_ = epoll_create(5);
  if (epoll_fd_ < 0) {
    std::cerr << "epoll_create failed " << strerror(errno) << std::endl;
    exit(1);
  }
  epoll_event event;
  event.data.fd = server_socket_;
  event.events = EPOLLIN;
  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, server_socket_, &event) < 0) {
    std::cerr << "epoll_ctl failed " << strerror(errno) << std::endl;
    exit(1);
  }

  std::thread(&UpdServer::run, this).join();
}

void UpdServer::run() {

  // One by One的处理数据报
  while (true) {
    recv_data();
    send_data();
  }
}

void UpdServer::recv_data() {
  if (packet_ == nullptr) {
    packet_ = new Packet();
    memset(packet_, 0, sizeof(Packet));
  }
  memset(buffer, 0, sizeof(buffer));
  client_addr_;
  socklen_t addr_len = sizeof(client_addr_);
  // 接收Packet的header部分
  int recv_data_size = recvfrom(server_socket_, buffer, MAX_UDP_DATA_SIZE, 0, (sockaddr*)&client_addr_, &addr_len);
  if (recv_data_size < 0) {
    std::cerr << "Recv data failed, " << strerror(errno) << std::endl;
    return;
  } else {
    memcpy(&packet_->header, buffer, sizeof(Packet::header));
    if (recv_data_size != sizeof(Packet) + packet_->header.data_size) {
      std::cerr << "Recv data failed, real recv data size: " << recv_data_size
                << ", need recv size: "
                << sizeof(Packet) + packet_->header.data_size << std::endl;
      return;
    } else {
      std::string recv_msg_str{buffer + 4};
      std::cout << "Udp Server recv client msg: " << recv_msg_str << std::endl;
    }
  }
}

void UpdServer::send_data() {
  int total_packs_size{0};
  int pack_header_size = sizeof(Packet::header);

  if (packet_ == nullptr) {
    packet_ = new Packet();
    memset(packet_, 0, sizeof(Packet));
  } else {
    total_packs_size = pack_header_size + packet_->header.data_size;
    memset(packet_, 0, total_packs_size);
  }

  std::string msg{"Hello Client, I'm Your Network Server"};
  if (total_packs_size < msg.size() + pack_header_size) {
    packet_ = (Packet*)realloc(packet_, msg.size() + pack_header_size);
  }
  strncpy(packet_->data, msg.c_str(), msg.size());
  packet_->header.data_size = msg.size();
  total_packs_size = pack_header_size + packet_->header.data_size;

  int send_data_size = sendto(server_socket_, (void*)packet_, total_packs_size, 0,
                              (sockaddr*)&client_addr_, sizeof(client_addr_));
  if (send_data_size != total_packs_size) {
    std::cerr << "Send response to client error, real send data size: "
              << send_data_size << ", need send size: " << total_packs_size
              << std::endl;
    return;
  } else {
    std::cout << "Send response to client: " << msg << std::endl; 
  }
}

}