#include "unix_socket/stream/client.h"

namespace unix_domain {
namespace stream {

Client::Client():link_info_{"/tmp/server.sock"}, sockfd_{-1} {
  memset(&server_addr_, 0, sizeof(server_addr_));
  server_addr_.sun_family = AF_LOCAL;
  // 必须保证sun_path数组以0结尾
  strncpy(server_addr_.sun_path, link_info_.c_str(), sizeof(server_addr_.sun_path) - 1);
}

Client::Client(std::string const& link_info):link_info_{link_info}, sockfd_{-1} {
  memset(&server_addr_, 0, sizeof(server_addr_));
  server_addr_.sun_family = AF_LOCAL;
  // 必须保证sun_path数组以0结尾
  strncpy(server_addr_.sun_path, link_info_.c_str(), sizeof(server_addr_.sun_path) - 1);
}

Client::~Client() {
  close(sockfd_);
  sockfd_ = -1;
}

void Client::Socket() {
  sockfd_ = socket(AF_LOCAL, SOCK_STREAM, 0);
  if (sockfd_ < 0) {
    std::cerr << "Create socket failed, " << strerror(errno) << std::endl;
    exit(1);
  }
  std::cout << "Create socket success\n";
}

void Client::Connect() {
  if (connect(sockfd_, (sockaddr*)&server_addr_, sizeof(server_addr_)) < 0) {
    std::cerr << "Connect to server failed, " << std::strerror(errno) << std::endl;
    exit(1); 
  } else {
    std::cout << "Connect to server success\n";
  }
}

void Client::Start() {
  send_data();
  recv_data();
}

void Client::Stop() {

}

void Client::send_data() {
  std::string hello_msg{"Hello Unix Domain Server"};
  memset(send_buffer, 0, sizeof(send_buffer));
  // 让pack指针指向定长数组，柔性数组就不再需要动态开辟内存
  Packet* pack = (Packet*)send_buffer;
  int pack_header_size = sizeof(Packet::header);

  pack->header.data_size = hello_msg.size();
  strncpy(pack->data, hello_msg.c_str(), hello_msg.size());

  msghdr msg;
  memset(&msg, 0, sizeof(msg));
  msg.msg_iovlen = 2;
  iovec iov[2];
  iov[0].iov_base = &pack->header;
  iov[0].iov_len = pack_header_size;
  iov[1].iov_base = pack->data;
  iov[1].iov_len = pack->header.data_size;
  msg.msg_iov = iov;

  int send_size{-1};
  if ((send_size = sendmsg(sockfd_, &msg, 0)) < 0) {
    std::cerr << "Sendmsg failed, " << strerror(errno) << std::endl;
    free(pack);
    exit(1);
  } else {
    std::cout << "Send " << hello_msg << " to server\n";
  }
  free(pack);
  pack = nullptr;
}

void Client::recv_data() {
  memset(recv_buffer, 0, sizeof(recv_buffer));
  // 让pack指针指向定长数组，柔性数组就不再需要动态开辟内存
  Packet* pack = (Packet*)recv_buffer;
  int pack_header_size{sizeof(Packet::header)};

  int recv_header_size = recvn(sockfd_, &pack->header, pack_header_size, MSG_PEEK);
  if (recv_header_size < 0) {
    std::cerr << "Peek pack header failed, " << strerror(errno) << std::endl;
    exit(1);
    free(pack);
  } else if (recv_header_size != pack_header_size) {
    std::cerr << "Peek pack header size error, real recv header size: "
              << recv_header_size << ", neet recv size: " << pack_header_size
              << std::endl;
    free(pack);
    exit(1);
  } else {
    std::cout << "Peek pack header success, pack data size is: " << pack->header.data_size << std::endl;
  }

  // recv pack data
  msghdr msg;
  memset(&msg, 0, sizeof(msg));
  msg.msg_iovlen = 2;
  iovec iov[2];
  iov[0].iov_base = &pack->header;
  iov[0].iov_len = pack_header_size;
  iov[1].iov_base = pack->data;
  iov[1].iov_len = pack->header.data_size;
  msg.msg_iov = iov;

  int recv_pack_size = recvmsgn(sockfd_, &msg, 0);
  if (recv_pack_size < 0) {
    std::cerr << "Recv pack faild, " << strerror(errno) << std::endl;
    exit(1);
    free(pack);
  } else if (recv_pack_size != pack_header_size + pack->header.data_size) {
    std::cerr << "Recv pack size error, real recv size: "
              << recv_pack_size << ", neet recv size: " << pack_header_size + pack->header.data_size
              << std::endl;
    exit(1);
    free(pack);
  } else {
    std::string pack_msg{pack->data};
    std::cout << "Recv pack success, pack msg: " << pack_msg << std::endl;
  }
  free(pack);
  pack = nullptr;
}

} // namespace stream
} // namespace unix_domain