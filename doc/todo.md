
# 開発計画
todoのようなもの

## 現在進行中の作業

## 全体的目標
未完
 - [ ] DoxygenでAPIドキュメントの作成
 - [ ] デバッグ情報がうまくつけれていない？
 - [ ] まずはシングルスレッドですべてを実装する
 - [ ] ICMPアプリケーションを動かす
 - [ ] UDPアプリケーションを動かす
 

完了済

## ARP
未完
 - [ ] 静的arpのみを利用するフラグを追加
 - [ ] arpresolv機能を追加

完了済
 - [x] 9/15 ARPテーブルの変な構造をシンプルにする
 - [x] 9/1  ioctlでARPテーブルの追加
 - [x] 9/15 ioctlでARPテーブルの消去
 - [x] 9/15 ioctlでARPテーブルの参照 

## Ether
未完
 - [ ] 9/20 ifnetとかからioctlでpromiscモードの設定を行う

完了済
 - [x] 9/13 ioctlでアドレスの設定
 - [x] 9/13 ioctlでアドレスの参照

## IP
未完
 - [ ] ioctlでnetmaskの設定
 - [ ] ioctlでnetmaskの参照
 - [ ] 
 - [ ] 静的arpを使ってパケット送信
 - [ ] 動的arpを使ってパケット送信
 - [ ] 
 - [ ] カーネルルーティングテーブルの実装
 - [ ] ioctlでルーティングレコードの設定
 - [ ] ioctlでルーティングレコードの参照

完了済
 - [x] 9/13 ioctlでアドレスの設定
 - [x] 9/13 ioctlでアドレスの参照

## ICMP
未完
 - [ ] icmpのパケット受信機能の実装
 - [ ] icmpのパケット送信機能の実装
 - [ ] icmp echo request/reply の実装
 - [ ] icmp destination unreachable の実装

完了済

## UDP
未完
 - [ ] UDPのポートが閉じている時の挙動を実装
 - [ ] UDPのポートが開いている時の挙動を実装

完了済

## Socketの実装
未完
 - [ ] sockaddrの種類を継承は保持か何で表現するか検討
 - [ ] aflink socketの実装
     - [ ] ioctlでインターフェースにバインド
	 - [ ] readでパケット受信
	 - [ ] writeでパケット送信
 - [ ] afinet raw socketの実装
	 - [ ] readでパケット受信
	 - [ ] writeでパケット送信
	 - [ ] ioctlでインターフェースにバインド
 - [ ] afinet raw socket sock-icmpの実装
     - [ ] 
 - [ ] afinet sockdgram socketの実装
     - [ ] openでポートとアドレスにバインド
	 - [ ] readでパケット受信
	 - [ ] writeでパケット送信

完了済
 - [x] 9/15 sockaddrのオペレータを実装する
 - [x] 9/15 アドレス関連をsockaddrで行う
 - [x] 9/15 各モジュールに大して、一番上から順にたどれば参照出きるようにする
