#ifndef PACKET_H_
#define PACKET_H_
#include <cstdint>
#include <string>
#include <unordered_map>

struct Head {
  uint32_t data_size;
};

struct Packet {
  Head header;
  char data[0]; // 柔性数组，可以根据payload的实际大小分配内存，并填充进去发送
};

// 局域网MTU为1500字节，减去IP头部20字节和UDP头部8字节，剩余1472字节
static constexpr uint32_t MAX_UDP_DATA_SIZE{1472};
// 互联网MTU为576字节，减去IP头部20字节和UDP头部8字节，剩余548字节
static constexpr uint32_t INTERNET_MAX_UDP_DATA_SIZE{548};

#endif  