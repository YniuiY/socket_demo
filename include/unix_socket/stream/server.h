#ifndef UNIX_DOMAIN_SERVER_H_
#define UNIX_DOMAIN_SERVER_H_

#include "common/headers.h"

namespace unix_domain {
namespace stream {

class Server {
 public:
  static constexpr int MAX_BUFFER_SIZE{65535};

  Server();
  explicit Server(std::string const& link_info);
  ~Server();

  void Socket();
  void Bind();
  void Listen();
  int Accept();
  void Start();
  void Stop();

 private:
  void run();
  int in_epoll_recvmsg(int sockfd);
  void in_epoll_sendmsg(int sockfd);

  int sockfd_;
  int epollfd_;
  sockaddr_un server_addr_;
  std::string link_info_;
  uint8_t recv_buffer[MAX_BUFFER_SIZE];
  uint8_t send_buffer[MAX_BUFFER_SIZE];
};

} // namespace stream
} // namespace unix_domain

#endif //UNIX_DOMAIN_SERVER_H_