

prefix = ..

INTERFACE  = eth0
# INTERFACE  = wlp3s0
# INTERFACE  = enp0s20u2
# INTERFACE  = eth1
# INTERFACE  = lo

TMUXSPLIT  = tmux split-window -v
TMUXVSPLIT = tmux split-window -h
TARGET = a.out
LOGNAME = log.log
SRCS   = \
		rte.cc \
		dpdk.cc \
		ifnet.cc \
		mbuf.cc \
		socket.cc \
		ethernet.cc \
		arp.cc \
		ip.cc \
		icmp.cc \
		stcp.cc \
		api.cc \
		main.cc 


OBJS   = $(SRCS:.cc=.o)


all: $(TARGET)

clean:
	$(RM) $(TARGET) $(OBJS) $(LOGNAME)

re: clean all


runc:
	sudo ./$(TARGET) \
		--vdev=eth_pcap0,iface=$(INTERFACE)

run:
	$(TMUXSPLIT) \
	sudo ./$(TARGET) \
		--vdev=eth_pcap0,iface=$(INTERFACE)

runall:
	$(TMUXSPLIT) \
	sudo ./$(TARGET) \
		--vdev=eth_pcap0,iface=eth0 \
		--vdev=eth_pcap1,iface=eth1 \
		--vdev=eth_pcap2,iface=lo 

gdb:
	sudo gdb -q --args \
		./$(TARGET) --vdev=eth_pcap0,iface=$(INTERFACE)

stat:
	$(TMUXVSPLIT) ./stat.zsh
	$(TMUXVSPLIT)  tail -f stcp.rx.log
	$(TMUXSPLIT)  tail -f stcp.tx.log


include $(prefix)/mk/vars.mk
LDFLAGS += -lslankdev -lpgen2

