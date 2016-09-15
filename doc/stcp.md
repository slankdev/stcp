
# STCP設計メモ

8/9のセキュリティキャンプまでにここにかかれた内容に関して実装し終わりたい。
目標としてIPアドレスでの通信が行なえること。


## メインスレッドの処理

```
while (true) {
	netifs_proc();

	arp.proc();
	ip.proc();
	icmp.proc();
	tcp.proc();

}
```


## どこまでDPDKの用意した機能に頼るか

関数や処理などはDPDKがかなり頑張ってくれるので、極力そっちを使う。
構造体に関してはじぶんでいじりやすくするため、自分でなるべく定義をして使っていく。

 - MACアドレスはDPDKのもの
 - それ以外はすべて自分で定義する


## netif

各ネットワークインターフェースからパケットの受信と送信を行う。
この辺はDPDKに任せっきり状態なので、中でどんな感じになっているかはおいおい勉強していかねば。。

```
for (net_device& dev : dpdk.devices) {
	uint16_t num_rx = dev.io_rx();
	if (unlikely(num_rx == 0)) return;

	// ここに適切なモジュールに必要な
	// 情報を付加してからデータを渡すコードを書く

	uint16_t num_reqest_to_send = dev.tx.size();
	uint16_t num_tx = dev.io_tx(num_reqest_to_send);
	if (num_tx != num_reqest_to_send)
		fprintf(stderr, "some packet droped \n");
}
```

## arp モジュール


### arpresolv() IPアドレスをHWアドレスに変換

これはIP層を考えることになってから考えるが、概要を以下に示しておく。

 	1. 送信先がbcastまたはmcastか確認
 	1. 既存の経路エントリの中にタイムアウトしていない完全な変換が存在し内科を確認
 	1. 存在すれば経路エントリないのゲートウェイの値が層審査期へのリンクアドレス
 	1. ARPリクエストを送信する。その間送信待ちをするパケットはARPエントリ待ちのキューに確保する


### ARPテーブル

 - ARPテーブル
	 - ARPエントリ
		 - HW address
		 - Proto address
		 - Interface


### 設計例

絶対厳密なARPの規格を違反しているが、現状はこれでいいかなあ。

```
class arptable_t {
	
};
class arp_mod {
	pktqueue_t rx;
	pktqueue_t wait_qeue;
	arptable_t table;

	arpresolv();
	flush_wait_queue();

public:
	proc();
};

void arp_mod::arpresolve(iface, mbuf, dstip)
{
	if (is_bcast(mbuf)) {
		set(0xffffffffffff);
		push_ifacetx(mbuf);
		return ;
	}
	if (hwaddr = exist_arp_table(dstip)) {
		set(hwaddr);
		push_ifacetx(mbuf);
		return ;
	}
	send_arp_request(mbuf, dstip);
	wait_qeue.push(mbuf);
}
void arp_mod::flush_wait_queue()
{
	size_t size=wait_qeue.size();
	for (size_t i=0; i<size; i++) {
		mbuf* m = wait_qeue.pop();
		dstip = get_dstip(m);
		if (hwaddr = exist_arp_table(dstip)) {
			set(hwaddr);
			push_tx(m);
			continue;
		}
		wait_qeue.push(m);
	}
}
void arp_mod::proc()
{
	if (is_arp_request) {
		if (arp.pdst == myipaddress) {
			reply_arp_reply();
		}
	} else if (is_arp_reply) {
		table.add();
	} else {
		warning("not support.");
	}
	flush_wait_queue();
}
```



## ip モジュール

まだ考えていない。



## STCPで使われる構造体、クラス概要

### クラス、構造体の区別

以下の規則をまもってクラスと構造体を使い分ける

 - ヘッダやアドレスは構造体で表現する
 - バイナリの相対アクセスを行なう場合のみの用途は構造体を使う
 - それ以外はクラスでオブジェクトとして考える


| クラス名      | 概要                       |
|:-------------:|:--------------------------:|
| class ifnet   | ネットワークインタフェース |
| class ifaddr  | アドレス                   |

```
clss ifnet {
public:
	std::vector<ifaddr> addrs;

	output();
	input();
	ioctl();
};

class ifaddr {
public:
	struct sockaddr* addr;
};
```


| 構造体名            | 概要                                                          |
|:-------------------:|:-------------------------------------------------------------:|
| struct arphdr       | ARPヘッダ                                                     |
| struct ether_header | Ethernetヘッダ                                                |
| struct iphdr        | IPヘッダ                                                      |
| struct tcphdr       | TCPヘッダ                                                     |
| struct ether_addr   | MACアドレス                                                   |
| struct in_addr      | IPアドレス                                                    |
| struct in_ifaddr    | ip,netmask,subnetaddrなどIPインターフェースごとのアドレス情報 |



