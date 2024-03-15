/**
 * 通常情况下tcp服务器是并发的，UDP服务器是迭代的
 * 所以一般来说UDP服务器不需要使用epoll这类I/O多路复用的方法。
 * 具体来看就是，UDP接收，发送数据始终用的同一个socket描述符，
 * 不存在TCP在accept后返回新链接socket描述符，使用新描述符通信的机制。
 */

#include "udp/udp_server.h"

namespace udp {

UpdServer::UpdServer():server_socket_{-1} {
  memset(&server_addr_, 0, sizeof(server_addr_));
  server_addr_.sin_family = AF_INET;
  server_addr_.sin_port = htons(51016);
  server_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
}

UpdServer::UpdServer(int const& port):server_socket_{-1} {
  memset(&server_addr_, 0, sizeof(server_addr_));
  server_addr_.sin_family = AF_INET;
  server_addr_.sin_port = htons(port);
  server_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
}

UpdServer::UpdServer(int const& port, std::string const& multi_cast_ip):server_socket_{-1} {
  memset(&server_addr_, 0, sizeof(server_addr_));
  server_addr_.sin_family = AF_INET;
  server_addr_.sin_port = htons(port);
  server_addr_.sin_addr.s_addr = htonl(INADDR_ANY);

  multi_cast_ip_ = multi_cast_ip;
}

UpdServer::~UpdServer() {
  close(server_socket_);
}

void UpdServer::Socket(bool is_multi_cast) {
  server_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
  if (server_socket_ == -1) {
    std::cerr << "Create udp socket failed\n";
    exit(1);
  } else {
    std::cout << "Create udp server socket success\n";
  }
  if (is_multi_cast && !multi_cast_ip_.empty()) {
    ip_mreq mreq;
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    // mreq.imr_multiaddr.s_addr = inet_addr(multi_cast_ip_.c_str());
    inet_pton(AF_INET, multi_cast_ip_.c_str(), &mreq.imr_multiaddr.s_addr);
    if (setsockopt(server_socket_, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
      std::cerr << "Add to multicast group failed, " << strerror(errno) << std::endl;
      exit(1);
    }
    std::cout << "Access multicast group, ip: " << multi_cast_ip_ << std::endl;
  }
}

void UpdServer::Bind() {
  bind(server_socket_, (sockaddr*)&server_addr_, sizeof(server_addr_));
  std::cout << "Upd server bind\n";
}

void UpdServer::Start() {
  std::cout << "Udp server start\n";
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
  memset(recv_buffer, 0, sizeof(recv_buffer));
  Packet* pack = reinterpret_cast<Packet*>(recv_buffer);

  client_addr_;
  socklen_t addr_len = sizeof(client_addr_);
  // 接收Packet的header部分
  int recv_data_size = recvfrom(server_socket_, pack, MAX_UDP_DATA_SIZE, 0, (sockaddr*)&client_addr_, &addr_len);
  if (recv_data_size < 0) {
    std::cerr << "Recv data failed, " << strerror(errno) << std::endl;
    return;
  } else {
    if (recv_data_size != sizeof(Packet) + pack->header.data_size) {
      std::cerr << "Recv data failed, real recv data size: " << recv_data_size
                << ", need recv size: "
                << sizeof(Packet) + pack->header.data_size << std::endl;
      return;
    } else {
      std::string recv_msg_str{pack->data};
      std::cout << "Udp Server recv client msg: " << recv_msg_str << std::endl;
    }
  }
}

void UpdServer::send_data() {
  memset(send_buffer, 0, MAX_UDP_DATA_SIZE);
  int total_packs_size{0};
  int pack_header_size = sizeof(Packet::header);
  Packet* pack = reinterpret_cast<Packet*>(send_buffer);
  std::string msg{"Hello Client, I'm Your Network Server"};

  strncpy(pack->data, msg.c_str(), msg.size());
  pack->header.data_size = msg.size();
  total_packs_size = pack_header_size + pack->header.data_size;

  int send_data_size = sendto(server_socket_, pack, total_packs_size, 0,
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