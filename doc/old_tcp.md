
# TCPに関する仕様

## States

 - TCPS_CLOSED
 - TCPS_LISTEN
 - TCPS_SYN_SENT
 - TCPS_SYN_RCVD
 - TCPS_ESTABLISHED
 - TCPS_FIN_WAIT_1
 - TCPS_FIN_WAIT_2
 - TCPS_CLOSE_WAIT
 - TCPS_CLOSING
 - TCPS_LAST_ACK
 - TCPS_TIME_WAIT


## イベント処理

発生するイベントを以下に示す

 - UserCall
    - OPEN
	- SEND
	- RECV
	- CLOSE
	- ABORT
	- STATUS
 - ReceiveSegment
    - ReceiveSegment
 - Timeout
    - User Timeout
	- Retransmission Timeout
	- TIME-WAIT Timeout


## ReceiveSegment

```
void ctrl_RST(seg):
	newseg = alloc_seg()
	newseg.seq = 0
	newseg.ack = seg.seq + seq.len
	newseg.flg = RSTACK
	return

void proc():
	seg = rcv_seg();
	switch (state):
		case TCPS_CLOSED:
			if RST:
				free(seg)
				return
			proc_RST(seg)
		case TCPS_LISTEN:
			if RST:
				proc_RST(seg)
			else if ACK:
				free(seg)
				return
			else if SYN:
				rcv_nxt    = seg.seq + 1
				irs        = seg.seq
				newseg     = alloc_seg()
				newseg.seq = iss
				newseg.ack = rcv_nxt
				newseg.flg = SYNACK

				snd_nxt = iss + 1
				snd_una = iss
				change_state(SYN-RCVD)
			else:
				throw exception("OKASHII")
		case TCPS_SYN_SENT:
			if ACK:
				if seg.ack <= iss or seg.ack > snd_nxt:
					seg.seg.ack
					seg.flg = RST
				if snd_una <= seg.ack <= snd_nxt
					nop()
			if RST
				print "error: connection reset"
				change_state(CLOSED)
		case TCPS_SYN_RCVD:
		case TCPS_ESTABLISHED:
		case TCPS_FIN_WAIT_1:
		case TCPS_FIN_WAIT_2:
		case TCPS_CLOSE_WAIT:
		case TCPS_CLOSING:
		case TCPS_LAST_ACK:
		case TCPS_TIME_WAIT:

			/*
			 * Check Sequemce
			 */
			if (seg.len==0 and seg.win==0) and (seg.seq = rcv_nxt)
				OK
			elif (seg.len==0 and seg.win>0) and (rcv_nxt <= seg.seq < rcv_nxt + rcv_wnd)
				OK
			elif (seg.len>0 and seg.win==0)
				NO
			elif (seg.len>0 and seg.win>0)
				if (rcv_nxt<=seg.seq<rcv_nxt+rcv_wnd) of
					(rcv_nxt <= seg.seq+seg.len-1 < rcv_nxt+rcv_wnd)
						OK
			else
				NO
				newseg = alloc_seg()
				newseg.seq = snd_nxt
				newseg.ack = rcv_nxt
				newseg.flg = ACK

			/*
			 * Check RST
			 */

```


## 参考文献

 - RFC793 jp http://www.f4.dion.ne.jp/~adem/rfc/rfc793.euc.txt
 -

