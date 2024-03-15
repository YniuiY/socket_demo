#ifndef PACKET_H_
#define PACKET_H_

struct Head {
  uint32_t data_size;
};

struct Packet {
  Head header;
  char data[0]; // 柔性数组，可以根据payload的实际大小分配内存，并填充进去发送
};

static constexpr uint32_t MAX_UDP_DATA_SIZE{1472 + 8};

#endif  