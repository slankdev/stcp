

#pragma once

#include <stcp/config.h>
#include <stcp/socket.h>
#include <stcp/stcp.h>
#include <stcp/util.h>


namespace slank {


enum tcpflag : uint8_t {
    TCPF_FIN  = 0x01<<0, /* 00000001 */
    TCPF_SYN  = 0x01<<1, /* 00000010 */
    TCPF_RST  = 0x01<<2, /* 00000100 */
    TCPF_PSH  = 0x01<<3, /* 00001000 */
    TCPF_ACK  = 0x01<<4, /* 00010000 */
    TCPF_URG  = 0x01<<5, /* 00100000 */
#if 0
    // nouse
    TCPF_SACK = 0x01<<6, /* 01000000 */
    TCPF_WACK = 0x01<<7, /* 10000000 */
#endif
};

enum socketstate {
    SOCKS_USE,
    SOCKS_UNUSE,
    SOCKS_WAITACCEPT,
};

enum tcpstate {
    TCPS_CLOSED      = 0,
    TCPS_LISTEN      = 1,
    TCPS_SYN_SENT    = 2,
    TCPS_SYN_RCVD    = 3,
    TCPS_ESTABLISHED = 4,
    TCPS_FIN_WAIT_1  = 5,
    TCPS_FIN_WAIT_2  = 6,
    TCPS_CLOSE_WAIT  = 7,
    TCPS_CLOSING     = 8,
    TCPS_LAST_ACK    = 9,
    TCPS_TIME_WAIT   = 10,
};


struct stcp_tcp_header {
	uint16_t sport   ; /* TCP source port.            */
	uint16_t dport   ; /* TCP destination port.       */
	uint32_t seq     ; /* TX data sequence number.    */
	uint32_t ack     ; /* RX data ack number.         */
	uint8_t  data_off; /* Data offset.                */
	uint8_t  flags   ; /* TCP flags                   */
	uint16_t rx_win  ; /* RX flow control window.     */
	uint16_t cksum   ; /* TCP checksum.               */
	uint16_t urp     ; /* TCP urgent pointer, if any. */

    void print() const
    {
        stcp_printf("TCP header \n");
        stcp_printf("+ sport    : %u 0x%04x \n", ntoh16(sport), ntoh16(sport));
        stcp_printf("+ dport    : %u 0x%04x \n", ntoh16(dport), ntoh16(dport));
        stcp_printf("+ seq num  : %u 0x%08x \n", ntoh32(seq  ), ntoh32(seq  ));
        stcp_printf("+ ack num  : %u 0x%08x \n", ntoh32(ack  ), ntoh32(ack  ));
        stcp_printf("+ data off : 0x%02x \n"   , data_off      );
        stcp_printf("+ tcp flags: 0x%02x \n"   , flags         );
        stcp_printf("+ rx win   : 0x%04x \n"   , ntoh16(rx_win)  );
        stcp_printf("+ cksum    : 0x%04x \n"   , ntoh16(cksum )  );
    }
};


struct tcpip {
    stcp_ip_header ip;
    stcp_tcp_header tcp;
    void print() const
    {
        ip.print();
        tcp.print();
    }
};


#if 0
enum tcp_op_number : uint8_t {
    TCP_OP_FIN = 0x00,
    TCP_OP_NOP = 0x01,
    TCP_OP_MSS = 0x02,
};
struct tcp_op_fin {
    uint8_t op_num;
};
struct tcp_op_nop {
    uint8_t op_num;
};
struct tcp_op_mss {
    uint8_t op_num;
    uint8_t op_len;
    uint16_t seg_siz;
};
#endif




class tcp_stream_info {
    /* HostByteOrder */
    uint32_t iss_    ; /* initial send sequence number      */
    uint32_t irs_    ; /* initial reseive sequence number   */

    uint32_t snd_nxt_; /* next send                         */
    uint16_t snd_win_; /* send window size                  */
    uint32_t snd_una_; /* unconfirmed send                  */
    uint32_t snd_wl1_; /* used sequence num at last send    */
    uint32_t snd_wl2_; /* used acknouledge num at last send */
    uint32_t rcv_nxt_; /* next receive                      */
    uint16_t rcv_wnd_; /* receive window size               */

#if 0
    uint16_t snd_up ; /* send urgent pointer               */
    uint16_t rcv_up ; /* receive urgent pointer            */
#endif

public:
    tcp_stream_info(uint32_t iss, uint32_t irs)
        : iss_(iss), irs_(irs), snd_win_(512) {}

    void iss_H(uint32_t arg) { iss_ = arg; }
    void iss_N(uint32_t arg) { iss_ = hton32(arg); }
    void irs_H(uint32_t arg) { irs_ = arg; }
    void irs_N(uint32_t arg) { irs_ = hton32(arg); }

    void snd_una_N(uint32_t arg)    { snd_una_ =  ntoh32(arg); }
    void snd_nxt_N(uint32_t arg)    { snd_nxt_ =  ntoh32(arg); }
    void snd_win_N(uint16_t arg)    { snd_win_ =  ntoh16(arg); }
    void snd_wl1_N(uint32_t arg)    { snd_wl1_ =  ntoh32(arg); }
    void snd_wl2_N(uint32_t arg)    { snd_wl2_ =  ntoh32(arg); }
    void rcv_nxt_N(uint32_t arg)    { rcv_nxt_ =  ntoh32(arg); }
    void rcv_win_N(uint16_t arg)    { rcv_wnd_ =  ntoh16(arg); }
    void snd_nxt_inc_N(int32_t arg) { snd_nxt_ += ntoh32(arg); }
    void rcv_nxt_inc_N(int32_t arg) { rcv_nxt_ += ntoh32(arg); }
    uint32_t iss_N() const     { return ntoh32(iss_    ); }
    uint32_t irs_N() const     { return ntoh32(irs_    ); }
    uint32_t snd_una_N() const { return ntoh32(snd_una_); }
    uint32_t snd_nxt_N() const { return ntoh32(snd_nxt_); }
    uint16_t snd_win_N() const { return ntoh16(snd_win_); }
    uint32_t snd_wl1_N() const { return ntoh32(snd_wl1_); }
    uint32_t snd_wl2_N() const { return ntoh32(snd_wl2_); }
    uint32_t rcv_nxt_N() const { return ntoh32(rcv_nxt_); }
    uint16_t rcv_win_N() const { return ntoh16(rcv_wnd_); }

    void snd_una_H(uint32_t arg)    { snd_una_ =  arg; }
    void snd_nxt_H(uint32_t arg)    { snd_nxt_ =  arg; }
    void snd_win_H(uint16_t arg)    { snd_win_ =  arg; }
    void snd_wl1_H(uint32_t arg)    { snd_wl1_ =  arg; }
    void snd_wl2_H(uint32_t arg)    { snd_wl2_ =  arg; }
    void rcv_nxt_H(uint32_t arg)    { rcv_nxt_ =  arg; }
    void rcv_win_H(uint16_t arg)    { rcv_wnd_ =  arg; }
    void snd_nxt_inc_H(int32_t arg) { snd_nxt_ += arg; }
    void rcv_nxt_inc_H(int32_t arg) { rcv_nxt_ += arg; }
    uint32_t iss_H() const     { return (iss_    ); }
    uint32_t irs_H() const     { return (irs_    ); }
    uint32_t snd_una_H() const { return (snd_una_); }
    uint32_t snd_nxt_H() const { return (snd_nxt_); }
    uint16_t snd_win_H() const { return (snd_win_); }
    uint32_t snd_wl1_H() const { return (snd_wl1_); }
    uint32_t snd_wl2_H() const { return (snd_wl2_); }
    uint32_t rcv_nxt_H() const { return (rcv_nxt_); }
    uint16_t rcv_win_H() const { return (rcv_wnd_); }
};


} /* namespace slank */
