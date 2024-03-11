#include "tcp/client.h"
#include "tcp/server.h"
#include "udp/udp_server.h"
#include "udp/udp_client.h"

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
      std::cout << "Usage: ./run tcp/udp server/client unix\n    Or ./run tcp/udp server/client\n";
      exit(1);
  }

  std::string h0{"tcp"};
  std::string h1{"udp"};
  std::string h2{"unix"};
  std::string h3{"server"};
  std::string h4{"client"};

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
    udp::UpdServer server(port);
    server.Socket();
    server.Bind();
    server.Start();
  } else if (!is_unix_possible && h1.compare(argv[1]) == 0 && h4.compare(argv[2]) == 0) {
    // udp client
    std::string ip{"172.17.50.226"};
    int port{51016};
    udp::UdpClient client(ip, port);
    client.Socket();
    client.Start();
  } else if (is_unix_possible && h0.compare(argv[1]) == 0 && h3.compare(argv[2]) == 0) {
    // unix stream server
  } else if (is_unix_possible && h1.compare(argv[1]) == 0 && h3.compare(argv[2]) == 0) {
    // unix data gram server
  } else if (is_unix_possible && h0.compare(argv[1]) == 0 && h4.compare(argv[2]) == 0) {
    // unix stream client
  } else if (is_unix_possible && h1.compare(argv[1]) == 0 && h4.compare(argv[2]) == 0) {
    // unix data gram client
  } else {
    std::cout << "Usage: ./run tcp/udp server/client unix or ./run tcp/udp server/client\n\n";
  }
  return 0;
}