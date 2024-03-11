#ifndef TRANSFER_H_
#define TRANSFER_H_

#include <sys/socket.h>

/// @brief 循环接收data_size字节的数据，还可以用flag为MSG_WAITALL的普通recv代替
/// @param fd socket描述符
/// @param vptr 应用层接收缓存区指针
/// @param data_size 需要接收的数据字节数
/// @param flag 暂未使用
/// @return 实际接收字节数, 出错返回-1
int recvn(int fd, void* vptr, int data_size, int flag);

/// @brief flag为MSG_WAITALL，接收到指定size的数据再返回的recv版本
/// @param fd socket描述符
/// @param vptr 应用层接收缓存区指针
/// @param data_size 需要接收的数据字节数
/// @return 实际接收字节数, 出错返回-1
int recvn(int fd, void* vptr, int data_size);

/// @brief 循环发送data_size字节的数据
/// @param fd socket描述符
/// @param vptr 应用层发送缓冲区指针
/// @param data_size 需要发送的数据字节数
/// @param flag 暂未使用
/// @return 实际发送字节数，出错返回-1
int sendn(int fd, void* vptr, int data_size, int flag);

/// @brief 完全接收指定字节数数据后再返回的recvmsg版本
/// @param sockfd socket描述符
/// @param msg 存储接收到数据的结构体指针
/// @param flag 无论传入什么都指定为MSG_WAITALL
/// @return 返回实际接收数据的字节数
int recvmsgn(int sockfd, msghdr* msg, int flag);

#endif //TRANSFER_H_