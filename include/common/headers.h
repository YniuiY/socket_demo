#ifndef HEADERS_H_
#define HEADERS_H_

#include <iostream>
#include <string>
#include <thread>
#include <sys/socket.h>
#include <sys/un.h> // unix domain socket
#include <arpa/inet.h> // inet_pton
#include <netinet/in.h> // sockaddr_in
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include <thread>

#include "common/packet.h"
#include "common/transfer.h"

#endif //HEADERS_H_