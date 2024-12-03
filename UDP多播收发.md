# UDP多播收发

## 1. 整体流程

### 1.1 发送者

	1. 创建套接字 socket( )
	2. 填充组播信息结构体 sockaddr_in
	3. 发送数据 sendto( )

### 1.2 接收者

	1.创建套接字 scoket( )
	2.填充组播信息结构体 sockaddr_in
	3.将套接字与组播信息结构体绑定 bind( )
	------------------------------------
	4.设置为加入多播组 setsockopt( )
	------------------------------------
	5.接收数据 recvfrom( )

----

## 2. 发送多播数据包

​	仅发送多播数据包不需要加入多播组，将目的地址设置为对应的多播地址即可，在Linux中，如果不设置发送对应多播地址使用的网卡，则会使用系统会使用路由表自动选择默认网卡进行发送（如果存在默认路由）。但通常多播数据包可能不是需要发送到默认路由，因此我们需要指定发送多播数据包使用的网卡，有以下三种方式：

1. 使用setsockopt通过IP_MULTICAST_IF选项设置发送数据包到特定多播地址时使用的对应网络接口地址。

```C++
struct ip_mreq mreq;
// 设置多播地址
mreq.imr_multiaddr.s_addr = inet_addr("226.1.11.111");
// 设置使用该多播地址发送数据时使用的网卡ip
mreq.imr_interface.s_addr = inet_addr("192.168.1.2");
// 调用setsockopt
ret = setsockopt(socks[i], IPPROTO_IP, IP_MULTICAST_IF, &mreq,sizeof(mreq));
if ( ret < 0) {
    LOG_ERROR("Fail to join udp group, err: %s", strerror(errno));
    goto out;
}
```

2. **使用setsockopt设置SO_BINDTODEVICE将该socket绑定到对应的网卡，此后通过该socket接收和发送的数据均只能通过绑定的网卡（需要root权限）。**

```C++
char* interface = "eth0";
ret = setsockopt(socks[i], SOL_SOCKET,SO_BINDTODEVICE, interface, strlen(interface));
if ( ret < 0 ) {
    LOG_ERROR("Fail to bind to interface, err: %s", strerror(errno));
    goto out;
}
```

3. 在路由表中指定对应多播地址使用的网卡地址（即将多播地址当作一个普通网段的地址，为该网段增加一个路由表项）。

```shell
route add -net 226.1.11.0 netmask 255.255.255.0 dev eth0
```

## 3. 接收多播数据包
接收多播数据包需要使用IP_ADD_MEMBERSHIP加入一个多播组。

```C++
// 加入组播
struct ip_mreq mreq;
// 设置需要接收的多播地址
mreq.imr_multiaddr.s_addr = inet_addr("226.1.11.111");
// 设置接收该多播地址数据的网卡ip
mreq.imr_interface.s_addr = inet_addr("192.168.1.2");
// 调用setsockopt
ret = setsockopt(socks[i], IPPROTO_IP, IP_ADD_MEMBERSHIP , &mreq,sizeof (mreq));
if ( ret < 0) {
    LOG_ERROR("Fail to join udp group, err: %s", strerror(errno));
    goto out;
}
```

如果需要一个socket仅接收特定网卡的多播数据包，在Linux中目前我找到的最好的办法就是设置SO_BINDTODEVICE选项将该socket绑定到指定网卡，如上一小节所述。



## 头文件

```C++
#include <sys/socket.h>  // socket 相关
#include <sys/un.h> // unix domain socket
#include <arpa/inet.h> // inet_pton
#include <netinet/in.h> // sockaddr_in ip地址相关
#include <sys/epoll.h> // epoll
```

