#include "tcp/client.h"
#include "common/transfer.h"

namespace tcp {

Client::Client():packet_{nullptr} {
  socket_ = -1;
  memset(&server_addr_, 0, sizeof(server_addr_));
  inet_pton(AF_INET, "127.0.0.1", &server_addr_.sin_addr);
  server_addr_.sin_port = htons(51015);
  server_addr_.sin_family = AF_INET;
}

Client::Client(std::string const& ip, int const& port):packet_{nullptr} {
  memset(&server_addr_, 0, sizeof(server_addr_));
  inet_pton(AF_INET, ip.c_str(), &server_addr_.sin_addr);
  server_addr_.sin_port = htons(port);
  server_addr_.sin_family = AF_INET;
}

Client::~Client() {
  close(socket_);
  if (packet_ != nullptr) {
    free(packet_);
    packet_ = nullptr;
  }
}

void Client::Socket() {
  socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_ == -1) {
    std::cerr << "Failed to create socket!" << std::endl;
    exit(1);
  } else {
    std::cout << "Create socket success!" << std::endl;
  }
}

void Client::Connect() {
  if (socket_ == -1) {
    std::cout << "Create socket first\n";
    exit(1);
  }
  if (connect(socket_, (sockaddr*)&(server_addr_), sizeof(server_addr_)) == -1) {
    std::cerr << "Faild to connect server!" << std::endl;
    exit(1);
  } else {
    std::cout << "Connect server success!" << std::endl;
  }
}

void Client::Send () {
  std::string msg{"Hello Server"};
  // 构造一个Packet
  int pack_size = sizeof(Packet) + msg.size();
  packet_ = (Packet*)malloc(pack_size);
  strcpy(packet_->data, msg.c_str());
  packet_->header.data_size = msg.size();
  printf("send msg: %s\n", packet_->data);
  int send_num = sendn(socket_, packet_, pack_size, 0);
  if (send_num != pack_size) {
    std::cerr << "Send byte: " << send_num << ", real byte: " << pack_size << std::endl;
    exit(1);
  } else {
    std::cout << "Send byte: " << send_num << ", real byte: " << pack_size << std::endl;
    Recv();
  }
}

void Client::Recv() {
  uint64_t pack_total_size{packet_->header.data_size + sizeof(Packet)};

  Head header;
  int recv_len = recv(socket_, &header, sizeof(Head), 0);
  if (recv_len == -1) {
    std::cerr << "Recv failed!" << std::endl;
    exit(1);
  } else {
    std::cout << "recv header data_size: " << (int)header.data_size << std::endl;
    if (header.data_size > packet_->header.data_size) {
      std::cout << "realloc pakcet_ to" << header.data_size + sizeof(Packet) << " byte" << std::endl;
      packet_ = (Packet*)realloc(packet_, header.data_size + sizeof(Packet));
      pack_total_size = header.data_size + sizeof(Packet);
    }
    memset(packet_, 0, pack_total_size);

    packet_->header.data_size = header.data_size;
    int data_len = recvn(socket_, packet_->data, packet_->header.data_size, 0);
    if (data_len == -1) {
      std::cerr << "Recv failed!" << std::endl;
      exit(1);
    } else {
      std::string msg{packet_->data};
      std::cout << "Recv msg: " << msg << std::endl;
    }
  }
}

void Client::SendMsg() {
  std::string msg{"Hello Server"};
  int total_pack_size = 0;
  Packet* pack{nullptr};
  pack = new Packet();
  total_pack_size = sizeof(Packet);
  if (pack->header.data_size < msg.size()) {
    pack = (Packet*)realloc(pack, sizeof(Packet::header) + msg.size());
    total_pack_size = sizeof(Packet::header) + msg.size();
  }
  memset(pack, 0, total_pack_size);
  strncpy(pack->data, msg.c_str(), msg.size());
  pack->header.data_size = msg.size();

  msghdr send_msg;
  memset(&send_msg, 0, sizeof(msghdr));
  send_msg.msg_iovlen = 2;
  iovec iov[2];
  iov[0].iov_base = &pack->header;
  iov[0].iov_len = sizeof(Packet::header);
  iov[1].iov_base = pack->data;
  iov[1].iov_len = pack->header.data_size;
  send_msg.msg_iov = iov;

  int nsend = -1;
  if ((nsend = sendmsg(socket_, &send_msg, 0)) < 0) {
    std::cerr << "sendmsg failed, " << strerror(errno) << std::endl;
    exit(1);
  } else if (nsend < total_pack_size) {
    std::cerr << "sendmsg size error, real send size: " << nsend << ", need send: " << total_pack_size << std::endl;
    exit(1);
  } else {
    std::cout << "sendmsg success\n";
    RecvMsg();
  }
}

void Client::RecvMsg() {
  int total_pack_size = -1;
  Packet* pack{nullptr};
  pack = new Packet();
  total_pack_size = sizeof(Packet);
  memset(pack, 0, total_pack_size);

  // peek 一下数据包的头部内容
  int recv_header_size = recvn(socket_, &pack->header, sizeof(Packet::header), MSG_PEEK);
  if (recv_header_size < 0) {
    std::cerr << "peek header failed, " << strerror(errno) << std::endl;
    return;
  } else if(recv_header_size == 0) {
    std::cout << "disconnected\n";
    return;
  } else if (recv_header_size != sizeof(Packet::header)) {
    std::cerr << "peek header size error, real recv size: " << recv_header_size
              << ", neet recv: " << sizeof(Packet::header) << std::endl;
    return;
  } else {
    std::cout << "peek pack header success, pack data size: " << pack->header.data_size << std::endl;
  }

  // 根据数据包头部的payload大小，开更大的内存用于接收完整的pack
  if (pack->header.data_size > total_pack_size - sizeof(Packet::header)) {
    pack = (Packet*)realloc(pack, pack->header.data_size + sizeof(Packet::header));
    total_pack_size = pack->header.data_size + sizeof(Packet::header);
  }

  // 接收完整的pack
  msghdr recv_msg;
  memset(&recv_msg, 0, sizeof(msghdr));
  recv_msg.msg_iovlen = 2;
  iovec iov[2];
  iov[0].iov_base = &pack->header;
  iov[0].iov_len = sizeof(Packet::header);
  iov[1].iov_base = pack->data;
  iov[1].iov_len = pack->header.data_size;
  recv_msg.msg_iov = iov;

  int nread = recvmsgn(socket_, &recv_msg, 0);
  if (nread < 0) {
    std::cerr << "recvmsgn failed, " << strerror(errno) << std::endl;
  } else if (nread != total_pack_size) {
    std::cerr << "recvmsgn size error, real recv size: " << nread << ", need recv size: " << total_pack_size << std::endl;
  } else {
    std::string msg{pack->data};
    std::cout << "recvmsgn success, recv msg: " << msg << std::endl;
  }

  if (pack != nullptr) {
    free(pack);
    pack = nullptr;
  }
}

}