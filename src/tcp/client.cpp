#include "tcp/client.h"

namespace tcp {

Client::Client() {
  socket_ = -1;
  memset(&server_addr_, 0, sizeof(server_addr_));
  inet_pton(AF_INET, "127.0.0.1", &server_addr_.sin_addr);
  server_addr_.sin_port = htons(51015);
  server_addr_.sin_family = AF_INET;
}

Client::Client(std::string const& ip, int const& port) {
  memset(&server_addr_, 0, sizeof(server_addr_));
  inet_pton(AF_INET, ip.c_str(), &server_addr_.sin_addr);
  server_addr_.sin_port = htons(port);
  server_addr_.sin_family = AF_INET;
}

Client::~Client() {
  close(socket_);
  free(packet_);
  packet_ = nullptr;
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
  int send_num = send(socket_, packet_, pack_size, 0);
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
    int data_len = recv(socket_, packet_->data, packet_->header.data_size, 0);
    if (data_len == -1) {
      std::cerr << "Recv failed!" << std::endl;
      exit(1);
    } else {
      std::string msg{packet_->data};
      std::cout << "Recv msg: " << msg << std::endl;
    }
  }
}

}