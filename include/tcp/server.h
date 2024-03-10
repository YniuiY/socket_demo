#ifndef SERVER_H_
#define SERVER_H_

#include "common/headers.h"
#include "common/transfer.h"

namespace tcp {

class Server {
 public:
  Server();
  explicit Server(int port);
  ~Server();

  void Socket();
  void Bind();
  void Listen();
  void Accept();

  void Start();
  void Stop();

  void Recv();
  void Send();

 private:
  void run();
  /// @brief 在epoll线程接收数据的函数
  /// @param accept进来的socket描述符
  /// @return 0表示对端连接中断，1表示接收正常
  int in_epoll_recv(int socket_fd);
  void in_epoll_send(int socket_fd);

  int sock_;
  int accept_sock_fd_;
  int epoll_fd_;
  Packet* packet_;
  sockaddr_in server_addr_;
};

}

#endif