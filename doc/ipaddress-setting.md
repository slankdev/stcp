
# IPアドレス設定関連

## コンセプト

現在socketシステムコールが実装されていないので、まずはそこを行うようになると思います。

## 予定タスク

以下のタスクを進める
 - stcp.socket()
 - stcp.ioctl()
 - stcp.xxxx()




## サンプルコード
このコードでIPアドレスが設定できるようにすることが目標です。

```
int
main()
{
 int fd;
 struct ifreq ifr;
 struct sockaddr_in *s_in;

 fd = socket(AF_INET, SOCK_DGRAM, 0);

 s_in = (struct sockaddr_in *)&ifr.ifr_addr;

 /* IPv4 */
 s_in->sin_family = AF_INET;
 /* 変更するIPアドレス */
 s_in->sin_addr.s_addr = inet_addr("10.1.2.30");

 /* IPアドレスを変更するインターフェースを指定 */
 strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

 /* IPアドレスを変更 */
 if (ioctl(fd, SIOCSIFADDR, &ifr) != 0) {
   /* 失敗したら理由を表示 */
   perror("ioctl");
 }

 close(fd);

 return 0;
}
```
