#include "unix_socket/stream/server.h"

namespace unix_domain {
namespace stream {

Server::Server():sockfd_{-1}, epollfd_{-1}, link_info_{"/tmp/server.sock"} {
  memset(&server_addr_, 0, sizeof(server_addr_));
  server_addr_.sun_family = AF_LOCAL;
  // 必须保证sun_path数组的最后一个字节是0
  strncpy(server_addr_.sun_path, link_info_.c_str(), sizeof(server_addr_.sun_path)-1);
}

Server::Server(std::string const& link_info):sockfd_{-1}, epollfd_{-1}, link_info_{link_info} {
  memset(&server_addr_, 0, sizeof(server_addr_));
  server_addr_.sun_family = AF_LOCAL;
  // 必须保证sun_path数组的最后一个字节是0
  strncpy(server_addr_.sun_path, link_info_.c_str(), sizeof(server_addr_.sun_path)-1);
}

Server::~Server() {
  close(sockfd_);
  sockfd_ = -1;
}

void Server::Socket() {
  sockfd_ = socket(AF_LOCAL, SOCK_STREAM, 0);
  if (sockfd_ < 0) {
    std::cerr << "Create socket failed, " << strerror(errno) << std::endl;
    exit(1);
  }
  std::cout << "Create socket success\n";
}

void Server::Bind() {
  unlink(link_info_.c_str());
  bind(sockfd_, (sockaddr*)&server_addr_, sizeof(server_addr_));
  std::cout << "Bind socket and link info\n";
}

void Server::Listen() {
  if (listen(sockfd_, 1000) < 0) {
    std::cerr << "Listen failed, " << strerror(errno) << std::endl;
    exit(1);
  }
}

void Server::Start() {
  // 创建epoll
  epollfd_ = epoll_create(5);
  if (epollfd_ < 0) {
    std::cerr << "Epoll create failed, " << strerror(errno) << std::endl;
    exit(1);
  }

  epoll_event event;
  event.data.fd = sockfd_;
  event.events = EPOLLIN | EPOLLET;

  if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, sockfd_, &event) < 0) {
    std::cerr << "Epoll ctl add sockfd failed, " << strerror(errno) << std::endl;
    exit(1);
  }

  std::thread(&Server::run, this).join();
}

void Server::Stop() {

}

int Server::Accept() {
  sockaddr_un client_addr;
  memset(&client_addr, 0, sizeof(client_addr));
  socklen_t addr_len = sizeof(client_addr);
  int client_sock = accept(sockfd_, (sockaddr*)&client_addr, &addr_len);
  if (client_sock < 0) {
    std::cerr << "accept client connect failed, " << strerror(errno) << std::endl;
    exit(1);
  }
  std::string client_path{client_addr.sun_path};
  std::cout << "accept client connect success, client addr: " << client_path << std::endl;
  return client_sock;
}

void Server::run() {
  static constexpr int MAX_EVENTS{6666};
  epoll_event events[MAX_EVENTS];
  int triggered_event_num = -1;
  int triggered_event_fd = -1;

  while(true) {
    triggered_event_num = epoll_wait(epollfd_, events, MAX_EVENTS, -1);
    for (int i = 0; i < triggered_event_num; ++i) {
      triggered_event_fd = events[i].data.fd;
      if (triggered_event_fd == sockfd_) {
        // 有客户端发起连接
        int client_sock = Accept();
        epoll_event event;
        event.data.fd = client_sock;
        event.events = EPOLLIN | EPOLLET;

        if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, client_sock, &event) < 0) {
          std::cerr << "epill_ctl add client sock failed, " << strerror(errno) << std::endl;
          exit(1);
        }
      } else if (events[i].events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR)) {
        // 连接出错，删除epoll对这个描述符的轮训，关闭描述符
        epoll_ctl(epollfd_, EPOLL_CTL_DEL, triggered_event_fd, nullptr);
        close(triggered_event_fd);
      } else if (events[i].events & EPOLLIN) {
        // 有描述符可读
        // 接收数据，将描述符epoll状态改为等待可写
        int recv_size = in_epoll_recvmsg(triggered_event_fd);
        if (recv_size == 0) {
          // 连接关闭
          if (epoll_ctl(epollfd_, EPOLL_CTL_DEL, triggered_event_fd, nullptr) < 0) {
            std::cerr << "epill_ctl delete client sock failed, " << strerror(errno) << std::endl;
            close(triggered_event_fd);
          }
        } else if (recv_size > 0) {
          epoll_event event;
          event.data.fd = triggered_event_fd;
          event.events = EPOLLOUT | EPOLLET;
          if (epoll_ctl(epollfd_, EPOLL_CTL_MOD, triggered_event_fd, &event) < 0) {
            std::cerr << "epill_ctl change client sock out failed, " << strerror(errno) << std::endl;
            exit(1);
          }
        } else {
          std::cerr << "recv msg failed, " << strerror(errno) << std::endl;
          exit(1);
        }

      } else if (events[i].events & EPOLLOUT) {
        // 有描述符可写
        // 发送数据，将描述符epoll状态恢复成等待可读
        in_epoll_sendmsg(triggered_event_fd);

        epoll_event event;
        event.data.fd = triggered_event_fd;
        event.events = EPOLLIN | EPOLLET;
        if (epoll_ctl(epollfd_, EPOLL_CTL_MOD, triggered_event_fd, &event) < 0) {
          std::cerr << "epill_ctl change client sock in failed, " << strerror(errno) << std::endl;
          exit(1);
        }
      }
    }
  }
}

int Server::in_epoll_recvmsg(int sockfd) {
  // 让pack指针指向定长数组，柔性数组就不再需要动态开辟内存
  Packet* pack = (Packet*)recv_buffer;
  memset(recv_buffer, 0, MAX_BUFFER_SIZE);
  int pack_header_size = sizeof(Packet::header);

  int recv_heade_size = recvn(sockfd, &pack->header, pack_header_size, MSG_PEEK);
  if (recv_heade_size < 0) {
    std::cerr << "Peek pack header failed, " << strerror(errno) << std::endl;
    return -1; 
  } else if (recv_heade_size != pack_header_size) {
    std::cerr << "Peek pack header size error, real recv header size: "
              << recv_heade_size << ", need recv size: " << pack_header_size
              << std::endl;
    return -1;
  } else {
    std::cout << "Peek pack header success, pack payload size: " << pack->header.data_size << std::endl;
  }

  int pack_data_size = pack->header.data_size;

  msghdr msg;
  memset(&msg, 0, sizeof(msg));
  msg.msg_iovlen = 2;
  iovec iov[2];
  iov[0].iov_base = &pack->header;
  iov[0].iov_len = pack_header_size;
  iov[1].iov_base = pack->data;
  iov[1].iov_len = pack_data_size;
  msg.msg_iov = iov;

  int recv_pack_size = recvmsgn(sockfd, &msg, 0);
  if (recv_pack_size < 0) {
    std::cerr << "Recvmsg pack failed, " << strerror(errno) << std::endl;
    free(pack);
    return -1; 
  } else if (recv_pack_size != pack_data_size + pack_header_size) {
    std::cerr << "Recvmsg pack size error, real recv size: "
              << recv_pack_size << ", need recv size: " << pack_header_size + pack_data_size
              << std::endl;
    free(pack);
    return -1;
  } else {
    std::string pack_msg{pack->data};
    std::cout << "Recvmsg pack success, pack msg: " << pack_msg << std::endl;
  }
  return recv_pack_size;
}

void Server::in_epoll_sendmsg(int sockfd) {
  int total_pack_size{0};
  memset(send_buffer, 0, MAX_BUFFER_SIZE);
  // 让pack指针指向定长数组，柔性数组就不再需要动态开辟内存
  Packet* pack = (Packet*)send_buffer;
  std::string send_msg_str{"Hello Unix Domain Client, I'm Your Local Server"};

  strcpy(pack->data, send_msg_str.c_str());
  pack->header.data_size = send_msg_str.size();
  total_pack_size = sizeof(Packet::header) + send_msg_str.size();

  msghdr send_msg;
  memset(&send_msg, 0, sizeof(msghdr));
  send_msg.msg_iovlen = 2;
  iovec iov[2];
  iov[0].iov_base = &pack->header;
  iov[0].iov_len = sizeof(Packet::header);
  iov[1].iov_base = pack->data;
  iov[1].iov_len = pack->header.data_size;
  send_msg.msg_iov = iov;
  int send_pack_size = sendmsg(sockfd, &send_msg, 0);
  if (send_pack_size != total_pack_size) {
    std::cerr << "Send pack failed, need send " << total_pack_size << " byte, real send: " << send_pack_size << " byte\n";
    exit(1);
  }
  std::cout << "Send response to client: " << send_msg_str << std::endl;
}

} // namesapce stream
} // namesapce unix_domain