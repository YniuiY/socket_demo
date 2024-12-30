#include "tcp/client.h"
#include "tcp/server.h"
#include "udp/piece_merge.h"
#include "udp/udp_server.h"
#include "udp/udp_client.h"
#include "unix_socket/stream/server.h"
#include "unix_socket/stream/client.h"
#include "common/big_string.h"
#include <cstdint>

// 多播IP，多播网卡接口使用INADDR_ANY，内核从主机现有网卡中分配一个，也可以明确指定
static const std::string MULTI_CAST_IP = "224.0.1.5";

int main (int argc, char* argv[]) {
  bool is_unix_possible{false};
  switch (argc) {
    case 3:
      is_unix_possible = false;
      break;
    case 4:
      is_unix_possible = true;
      break;
    default:
      std::cout << "Usage: ./run tcp/udp server/client unix\n    Or ./run tcp/udp server/client/merge\n";
      exit(1);
  }

  std::string h0{"tcp"};
  std::string h1{"udp"};
  std::string h2{"unix"};
  std::string h3{"server"};
  std::string h4{"client"};
  std::string h5{"merge"};

  if (!is_unix_possible && h0.compare(argv[1]) == 0 && h3.compare(argv[2]) == 0) {
    // tcp server
    int port{51015};
    tcp::Server server(port);
    server.Socket();
    server.Bind();
    server.Listen();
    server.Start();
  } else if (!is_unix_possible && h0.compare(argv[1]) == 0 && h4.compare(argv[2]) == 0) {
    // tcp client
    std::string ip{"10.1.2.147"};
    int port{51015};
    tcp::Client client(ip, port);
    client.Socket();
    client.Connect();
    // client.Send();
    client.SendMsg();
  } else if (!is_unix_possible && h1.compare(argv[1]) == 0 && h3.compare(argv[2]) == 0) {
    //udp server
    int port{51016};
    bool is_access_multi_cast{true};
    // udp::UpdServer server(port);
    // 加入多播组的服务端，具备了接收多播报文的能力
    udp::UpdServer server(port, MULTI_CAST_IP);
    server.Socket(is_access_multi_cast);
    server.Bind();
    server.Start();
  } else if (!is_unix_possible && h1.compare(argv[1]) == 0 && h4.compare(argv[2]) == 0) {
    // udp client
    bool is_broadcast = false;
    std::string ip{"10.1.2.255"};
    // 客户端向多播地址发送数据
    ip = MULTI_CAST_IP;
    int port{51016};
    udp::UdpClient client(ip, port);
    client.Socket(is_broadcast);
    client.Start();
  } else if (is_unix_possible && h0.compare(argv[1]) == 0 && h3.compare(argv[2]) == 0) {
    // unix stream server
    std::string link_info{"/tmp/server.sock"};
    unix_domain::stream::Server server(link_info);
    server.Socket();
    server.Bind();
    server.Listen();
    server.Start();
    server.Stop();
  } else if (is_unix_possible && h0.compare(argv[1]) == 0 && h4.compare(argv[2]) == 0) {
    // unix stream client
    std::string link_info{"/tmp/server.sock"};
    unix_domain::stream::Client client(link_info);
    client.Socket();
    client.Connect();
    client.Start();
    client.Stop();
  } else if (is_unix_possible && h1.compare(argv[1]) == 0 && h3.compare(argv[2]) == 0) {
    // unix data gram server
  } else if (is_unix_possible && h1.compare(argv[1]) == 0 && h4.compare(argv[2]) == 0) {
    // unix data gram client
  } else if (!is_unix_possible && h1.compare(argv[1]) == 0 && h5.compare(argv[2]) == 0){
    // UDP子包合并测试
    uint8_t* merged_buffer_ptr{nullptr};
    std::string big_string = BIG_STR;
    std::string littel_string = LITTEL_STR;
    udp::piece_merge::PieceMerge piece_merge;
    std::vector<udp::piece_merge::SubPkt> sub_pkts = piece_merge.CutPackage((uint8_t*)littel_string.c_str(), littel_string.size(), "114514");
    for (auto &sub_pkt : sub_pkts) {
      std::cout << "sub pkt index:" << sub_pkt.header.index << std::endl;
      if (sub_pkt.header.index == 0) {
        merged_buffer_ptr = new uint8_t[sub_pkt.header.total_size];
        std::cout << "pkg total size: " << sub_pkt.header.total_size << std::endl;
      }
      if (piece_merge.MergeSubPackge(&sub_pkt, merged_buffer_ptr, "114514")) {
        std::cout << "merged sub pakge success" << std::endl;
        std::string str((char*)merged_buffer_ptr);
        std::cout << "merged string: " << str << std::endl;
      } else {
        std::cout << "sub pakge mergeing, sub inde: " << sub_pkt.header.index << ", total index: " << sub_pkt.header.comm_header.total_sub_pkt_num << std::endl;
      }
    }

  } else {
    std::cout << "Usage: ./run tcp/udp server/client unix or ./run tcp/udp server/client/merge\n\n";
  }
  return 0;
}