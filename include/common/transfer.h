#ifndef TRANSFER_H_
#define TRANSFER_H_

/// @brief 循环接收data_size字节的数据
/// @param fd socket描述符
/// @param vptr 应用层接收缓存区指针
/// @param data_size 需要接收的数据字节数
/// @param flag 暂未使用
/// @return 实际接收字节数, 出错返回-1
int recvn(int fd, void* vptr, int data_size, int flag);

/// @brief 循环发送data_size字节的数据
/// @param fd socket描述符
/// @param vptr 应用层发送缓冲区指针
/// @param data_size 需要发送的数据字节数
/// @param flag 暂未使用
/// @return 实际发送字节数，出错返回-1
int sendn(int fd, void* vptr, int data_size, int flag);

#endif //TRANSFER_H_