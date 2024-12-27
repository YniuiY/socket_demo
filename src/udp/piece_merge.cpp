#include "udp/piece_merge.h"
#include "common/packet.h"
#include <cstdint>

namespace udp {
namespace piece_merge{

std::vector<SubPkt> PieceMerge::CutPackage(uint8_t* data, uint32_t data_size, std::string const& client_addr) {
  std::vector<SubPkt> sub_pkts;
  SubPkt sub_pkt;
  // 计算分片数量
  uint32_t sub_pkt_nums = (data_size % MAX_UDP_DATA_SIZE) > 0? data_size/MAX_UDP_DATA_SIZE + 1 : data_size/MAX_UDP_DATA_SIZE;
  if (sub_pkt_nums <= 1) {
    // 数据不分片
    sub_pkt.header.comm_header.msg_type = KNON_PIECE;
    sub_pkts.emplace_back(sub_pkt);
  } else {
    // 最后一片的大小
    uint32_t last_pkt_size = (data_size % MAX_UDP_DATA_SIZE);
    // 获取目的地址的序列号
    if (multi_point_seq_map_.find(client_addr) == multi_point_seq_map_.end()) {
      multi_point_seq_map_[client_addr] = 0;
    } else {
      multi_point_seq_map_[client_addr]++;
    }
    uint32_t seq = multi_point_seq_map_.at(client_addr);
    uint32_t sub_pkt_index = 0;

    for (int i = 0; i < sub_pkt_nums; ++i) {
      sub_pkt.header.comm_header.msg_type = KPIECE;
      sub_pkt.header.seq = seq;
      sub_pkt.header.index = sub_pkt_index++;
      sub_pkt.header.total_size = data_size;
      sub_pkt.header.offset = i * MAX_UDP_DATA_SIZE;
      sub_pkt.header.payload_size = MAX_UDP_DATA_SIZE;
      if (i == sub_pkt_nums-1 && last_pkt_size) {
        // 最后一片的payload大小和偏移分量
        sub_pkt.header.payload_size = last_pkt_size;
        sub_pkt.header.offset = data_size - last_pkt_size;
      }
      // 指针指向此分片对应的地址
      sub_pkt.payload = data + sub_pkt.header.offset;
      sub_pkts.emplace_back(sub_pkt);
    }
  }

  return sub_pkts;
}



} // namespace piece_merge
} // namespace udp