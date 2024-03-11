#ifndef CLIENT_H_
#define CLIENT_H_

#include "common/headers.h"

namespace tcp {

class Client {
 public:
  Client();
  Client(std::string const& ip, int const& port);
  ~Client();

  void Socket();
  void Connect();
  void Send();
  void Recv();
  void SendMsg();
  void RecvMsg();

 private:
  int socket_;
  sockaddr_in server_addr_; // 服务器地址
  Packet* packet_;
};

}
#endif