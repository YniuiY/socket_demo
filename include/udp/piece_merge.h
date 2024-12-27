#ifndef UDP_PIECE_MERGE_H
#define UDP_PIECE_MERGE_H

/*
 UDP分包组包算法模块
*/

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

#include "common/packet.h"

namespace udp {
namespace piece_merge{

static constexpr uint32_t KPIECE{1130};
static constexpr uint32_t KNON_PIECE{1125};

struct CommonHeader {
  uint32_t msg_type; // 1130表示分片，1125表示未分片
};

// 子包头部信息
struct SubPktHeader {
  CommonHeader comm_header; // 公共包头
  uint32_t seq;             // 整包序列号
  uint32_t index;           // 子包序号
  uint32_t payload_size;    // 子包数据大小
  uint32_t total_size;      // 整包数据大小
  uint32_t offset;          // 子包在整包中的偏移量
};

struct SubPkt {
  SubPktHeader header;
  uint8_t* payload;
};

static constexpr uint32_t HEADER_SIZE{sizeof(SubPktHeader)};

// 子包合并缓存区
struct PktMerge {
  uint32_t seq;           // 整包序列号
  uint32_t sub_pkt_num;   // 子包数量
  uint32_t total_size;    // 整包数据大小
  uint8_t* merge_buffer;  // 整包合并缓冲区
};

class PieceMerge {
 public:
  PieceMerge() = default;
  ~PieceMerge() = default;

  /// @brief 切割子包
  /// @param data 子包数据
  /// @param data_size 子包数据大小
  /// @param client_addr 子包发送目的地址
  /// @return 切割后的子包信息
  std::vector<SubPkt> CutPackage(uint8_t* data, uint32_t data_size, std::string const& client_addr);

  /// @brief 合并子包
  /// @param sub_pkt 子包信息
  /// @param data 合并后的整包数据，合并前nullptr
  /// @param client_addr 子包发送端地址
  /// @return 合并成功返回true，否则返回false
  bool MergeSubPackge(SubPkt* sub_pkt, uint8_t* data, std::string const& client_addr);

 private:
  // 用于存储多端点的合并信息, key为端点地址{地址:端口 或 ipc绝对地址} value为PktMerge
  std::unordered_map<std::string, PktMerge> multi_point_merge_map_;
  // 用于存储多端点的序列号, key为端点地址{地址:端口 或 ipc绝对地址} value为序列号，保证同一个目的地址序列号唯一
  std::unordered_map<std::string, uint32_t> multi_point_seq_map_;
};

} // namespace piece_merge
} // namespace udp

#endif // UDP_PIECE_MERGE_H