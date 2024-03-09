#include "tcp/server.h"

namespace tcp {

Server::Server(): packet_{nullptr}, epoll_fd_{-1} {
  memset(&server_addr_, 0, sizeof(server_addr_));
  server_addr_.sin_family = AF_INET;
  server_addr_.sin_port = htons(51015);
  server_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
}

Server::Server(int port): packet_{nullptr}, epoll_fd_{-1} {
  memset(&server_addr_, 0, sizeof(server_addr_));
  server_addr_.sin_family = AF_INET;
  server_addr_.sin_port = htons(port);
  server_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
}

Server::~Server() {
  close(sock_);
  close(epoll_fd_);
  // todo: 在Server析构的时候，将accept进来但还未关闭的socket关闭
  free(packet_);
  packet_ = nullptr;
}

void Server::Socket() {
  sock_ = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_ == -1) {
    std::cerr << "socket create faild" << std::endl;
    exit(1);
  } else {
    std::cout << "Socket create success\n";
  }
}

void Server::Bind() {
  bind(sock_, (sockaddr*)&(server_addr_), sizeof(server_addr_));
  std::cout << "Bind server ip and port to socket fd\n";
}

void Server::Listen() {
  if (listen(sock_, SOMAXCONN)== -1) {
    std::cerr << "Listen failed\n";
    exit(1);
  } else {
    std::cout << "Listen success\n";
  }
}

void Server::Accept() {
  uint32_t addr_len = sizeof(server_addr_);
  accept_sock_fd_ = accept(sock_, (sockaddr*)&(server_addr_), &addr_len);
  if (accept_sock_fd_ == -1) {
    std::cerr << "Accept connect failed\n";
    exit(1);
  } else {
    std::cout << "Accept connect success\n";
  }
}

void Server::Recv() {
  //构造基础packet
  packet_ = new Packet();
  packet_->header.data_size = 0;

  // 接收header
  int recv_header_size = recv(accept_sock_fd_, (void*)&(packet_->header), sizeof(Head), 0);
  if (recv_header_size == -1) {
    std::cerr << "Recv data failed\n";
    exit(1);
  } else {
    std::cout << "Recv client pack header size: " << recv_header_size
              << ", pack data size: " << packet_->header.data_size
              << std::endl;
    // 根据header信息构造pack
    int total_pack_size = packet_->header.data_size + sizeof(Packet);
    packet_ = (Packet*)realloc(packet_, total_pack_size);
    // 接收pack数据
    int recv_data_size = recv(accept_sock_fd_, (void*)&(packet_->data), packet_->header.data_size, 0);
    if (recv_data_size == -1) {
      std::cerr << "Recv data failed\n";
      exit(1);
    } else {
      std::string msg{packet_->data};
      std::cout << "Recv cleint msg: " << msg << std::endl;
      Send();
    }
  }
}

void Server::Send() {
  uint64_t pack_total_size{packet_->header.data_size + sizeof(Packet)};
  // std::string send_msg{"Hello Client, I'm Your Network Server"};
  std::string send_msg{"Hello!"};
  // 填充数据包
  if (packet_->header.data_size < send_msg.size()) {
    packet_ = (Packet*)realloc(packet_, send_msg.size() + sizeof(Packet));
    pack_total_size = sizeof(Packet) + send_msg.size();
  }
  memset(packet_, 0, pack_total_size);
  packet_->header.data_size = send_msg.size();
  strcpy(packet_->data, send_msg.c_str());
  // 发送数据包
  int send_num = send(accept_sock_fd_, packet_, pack_total_size, 0);
  if (send_num == -1) {
    std::cerr << "Send data failed\n";
    exit(1);
  }
}

void Server::Start() {
  // todo: 启动一个线程作为epoll的线程，处理client接入后的通信工作
  // 创建epoll fd
  epoll_fd_ = epoll_create(6);
  if (epoll_fd_ == -1) {
    std::cerr << "epoll_create failed\n";
    exit(1);
  }

  epoll_event event;
  event.data.fd = sock_;
  // 表示描述符可读 | 边沿触发 
  event.events = EPOLLIN | EPOLLET;

  if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, sock_, &event) != 0) {
    std::cerr << "epoll_ctl failed\n";
    exit(1);
  }

  std::thread(&Server::run, this).join();
}

void Server::run() {
  static constexpr uint32_t MAX_EVENT_NUM{8650};
  epoll_event events[MAX_EVENT_NUM];
  
  while (true) {
    int triggered_event_num = epoll_wait(epoll_fd_, events, MAX_EVENT_NUM, -1);
    if (triggered_event_num == -1) {
      std::cerr << "epoll_wait failed, " << strerror(errno) << std::endl;
      break;
    }

    for (int i = 0; i < triggered_event_num; ++i) {
      int triggered_socket_fd = events[i].data.fd; // 被触发的socket fd

      if (triggered_socket_fd == sock_) {
        // listen被触发，有client接入，
        // 调用accept建立连接，将accept返回的socket加入epoll中，等待可读
        sockaddr_in client_sockaddr;
        bzero(&client_sockaddr, sizeof(client_sockaddr));
        uint32_t addr_len{sizeof(client_sockaddr)};
        // todo: 在Server析构的时候，将accept进来但还未关闭的socket关闭
        int client_socket = accept(triggered_socket_fd, (sockaddr*)&(client_sockaddr), &addr_len);
        if (client_socket == -1) {
          std::cerr << "accept client connect failed\n";
          exit(1);
        }
        // 打印客户端的ip和端口
        char client_ip[64];
        inet_ntop(AF_INET, &client_sockaddr.sin_addr, client_ip, sizeof(client_sockaddr));
        std::string client_ip_str{client_ip};
        std::cout << "client addr: " << client_ip_str << ", client port: " << ntohs(client_sockaddr.sin_port) << std::endl;

        epoll_event event;
        event.data.fd = client_socket;
        event.events = EPOLLIN | EPOLLET;

        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_socket, &event) != 0) {
          std::cerr << "epoll add client socket failed\n";
          exit(1);
        }
      } else if (events[i].events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR)) {
        // 发生错误，连接断开
        epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, triggered_socket_fd, 0);
        close(triggered_socket_fd);

      } else if (events[i].events & EPOLLIN) {
        // Client有数据到达，接收处理数据
        if (in_epoll_recv(events[i].data.fd)) {
          // 将这个触发的socket，在epoll中变为等待可写，再次触发后发送数据给client
          epoll_event event;
          event.data.fd = events[i].data.fd;
          event.events = EPOLLOUT | EPOLLET;
          epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, events[i].data.fd, &event);
        } else {
          // 在epoll中删除对端断开连接的socket描述符，并关闭这个socket描述符
          int disconnected_socket_fd = events[i].data.fd;
          if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, events[i].data.fd, nullptr) == -1) {
            std::cerr << "epoll_ctl delete disconnected socket fd failed\n";
          } else {
            std::cout << "epoll_ctl delete disconnected socket fd success\n";
          }
          close(disconnected_socket_fd); // 服务端也关闭这条连接的socket描述符
        }
      } else if (events[i].events & EPOLLOUT) {
        // 发送数据到Client
        in_epoll_send(events[i].data.fd);

        // 数据发送完成后，将这个socket在epoll中恢复成等待可读
        epoll_event event;
        event.data.fd = events[i].data.fd;
        event.events = EPOLLIN | EPOLLET;
        epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, events[i].data.fd, &event);
      }
    }
  }
}

int Server::in_epoll_recv(int socket_fd) {
  int ret{1};
  int total_pack_size{0};
  if (packet_ == nullptr) {
    packet_ = new Packet();
  } else {
    total_pack_size = packet_->header.data_size + sizeof(Packet::header);
    memset(packet_, 0, total_pack_size);
  }

  int recv_header_size = recv(socket_fd, &packet_->header.data_size, sizeof(Packet), 0);
  if (recv_header_size != 0 && recv_header_size != sizeof(Packet::header)) {
    std::cerr << "[in_epoll_recv], recv header size: " << recv_header_size
              << ", not equal to " << sizeof(Packet::header) << std::endl;
    exit(1);
  } else if (recv_header_size == 0) {
    // 对端连接中断
    std::cout << "[in_epoll_recv], client close connect\n";
    ret = 0;
    return ret;
  }
  std::cout << "[in_epoll_recv] recv header " << recv_header_size << " byte\n";
  total_pack_size = packet_->header.data_size + sizeof(Packet::header);
  packet_ = (Packet*)realloc(packet_, total_pack_size);
  // 接收pack的数据部分
  int recv_pack_data_size = recv(socket_fd, packet_->data, packet_->header.data_size, 0);
  if (recv_pack_data_size != packet_->header.data_size) {
    std::cout << "[in_epoll_recv] recv pack data failed, need recv size: "
              << packet_->header.data_size
              << ", real recv size: " << recv_pack_data_size << std::endl;
    exit(1);
  }
  std::string msg_str{packet_->data};
  std::cout << "Recv client pack msg: " << msg_str << std::endl;
  return ret;
}

void Server::in_epoll_send(int socket_fd) {
  int total_pack_size{0};
  if (packet_ == nullptr) {
    packet_ = new Packet();
  } else {
    total_pack_size = packet_->header.data_size + sizeof(Packet::header);
    memset(packet_, 0, total_pack_size);
  }
  std::string send_msg_str{"Hello Client, I'm Your Network Server"};
  if (packet_->header.data_size < send_msg_str.size()) {
    packet_ = (Packet*)realloc(packet_, sizeof(Packet::header) + send_msg_str.size());
  }
  strcpy(packet_->data, send_msg_str.c_str());
  packet_->header.data_size = send_msg_str.size();

  total_pack_size = sizeof(Packet::header) + send_msg_str.size();
  int send_pack_size = send(socket_fd, packet_, total_pack_size, 0);
  if (send_pack_size != total_pack_size) {
    std::cerr << "Send pack failed, need send " << total_pack_size << " byte, real send: " << send_pack_size << " byte\n";
    exit(1);
  }
  std::cout << "Send response to client: " << send_msg_str << std::endl;
}

}