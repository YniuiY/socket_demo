#include "common/transfer.h"

#include "common/headers.h"

int recvn(int fd, void* vptr, int data_size, int flag) {
  int nleft = data_size;
  ssize_t nread = 0;
  void* ptr = vptr;

  while (nleft > 0) {
    if ((nread = recv(fd, ptr, nleft, flag)) < 0) {
      if (errno == EINTR) {
        nread = 0;  // 重新开始接收
      } else {
        return -1;
      }
    } else if (nread == 0) {
      break;  // 链接中断
    }

    nleft -= nread;
    ptr += nread;
  }

  return (data_size - nleft);
}

int recvn(int fd, void* vptr, int data_size) {
  return recv(fd, vptr, data_size, MSG_WAITALL);
}

int sendn(int fd, void* vptr, int data_size, int flag) {
  int nleft = data_size;
  ssize_t nsended = 0;
  void* ptr = vptr;

  while (nleft > 0) {
    if ((nsended = send(fd, vptr, nleft, 0)) <= 0) {
      if (nsended < 0 && errno == EINTR) {
        nsended = 0;  // 重新开始发送
      } else {
        return -1;
      }
    }

    nleft -= nsended;
    ptr += nsended;
  }

  return data_size;
}

int recvmsgn(int sockfd, msghdr* msg, int flag) {
  return recvmsg(sockfd, msg, MSG_WAITALL);
}