

# Sockaddr構造体によるアドレスの管理、設定方法

伝統的手法ではsockaddr構造体のポインタを中心に様々な構造体を渡しているが、
せっかくC++で開発をしているので、sockaddrクラスといった感じにして
sockaddr_inやsockaddr_dlはsockaddrクラスを継承して実装する方法を採用しようと思う。

このドキュメントでは外部実装から説明をしていく

## アドレスの設定方法
伝統的方法ではioctlを使ってアドレス設定を行ってる。setsockoptとかもあるかもしれないけど、
そんな高機能なやつは現段階ではいらない。必要最低限でシンプルな実装を目指している。


一般的なc言語での実装を以下に示す。

```
#include <stdio.h>
#include <string.h> /* for strncpy */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

int main()
{

 int fd = socket(AF_INET, SOCK_DGRAM, 0);

 struct ifreq ifr;
 struct sockaddr_in *s_in = (struct sockaddr_in *)&ifr.ifr_addr;

 s_in->sin_family = AF_INET;
 s_in->sin_addr.s_addr = inet_addr("10.1.2.30");

 strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

 if (ioctl(fd, SIOCSIFADDR, &ifr) != 0) {
   perror("ioctl");
 }

 close(fd);
 return 0;
}
```

現段階ではこの様にしてipアドレスを設定出きるようにする。(予定)

```
int set_address_main()
{
	dpdk& d = dpdk::instance();
	ifnet& ifn = d.devices[0];

	struct 

	ifn.ioctl(SIOCSIFADDR, &ifr);
}

```

