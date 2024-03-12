#ifndef UNIX_DOMAIN_CLIENT_H_
#define UNIX_DOMAIN_CLIENT_H_

#include "common/headers.h"

namespace unix_domain {
namespace stream {

class Client {
 public:
  static constexpr int MAX_BUFFER_SIZE{65535};

  Client();
  explicit Client(std::string const& link_info);
  ~Client();

  void Socket();
  void Connect();
  void Start();
  void Stop();

 private:
  void run();
  void send_data();
  void recv_data();

  std::string link_info_;
  int sockfd_;
  sockaddr_un server_addr_;
  uint8_t recv_buffer[MAX_BUFFER_SIZE];
  uint8_t send_buffer[MAX_BUFFER_SIZE];
};

} // namespace stream
} // namespace unix_domain

#endif //UNIX_DOMAIN_CLIENT_H_