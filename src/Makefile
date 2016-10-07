

prefix = ..

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



run:
	$(TMUXSPLIT) \
	sudo ./$(TARGET) \
		--vdev=eth_pcap0,iface=eth0

runenp:
	$(TMUXSPLIT) \
	sudo ./$(TARGET) \
		--vdev=eth_pcap0,iface=enp0s20u2

runwl:
	$(TMUXSPLIT) \
	sudo ./$(TARGET) \
		--vdev=eth_pcap0,iface=wlp3s0

runlo:
	$(TMUXSPLIT) \
	sudo ./$(TARGET) \
		--vdev=eth_pcap0,iface=lo

runall:
	$(TMUXSPLIT) \
	sudo ./$(TARGET) \
		--vdev=eth_pcap0,iface=eth0 \
		--vdev=eth_pcap1,iface=eth1 \
		--vdev=eth_pcap2,iface=lo 

gdb:
	$(TMUXSPLIT) \
	sudo gdb -q --args \
		./$(TARGET) --vdev=eth_pcap0,iface=eth0

gdblo:
	$(TMUXSPLIT) \
	sudo gdb -q --args \
		./$(TARGET) --vdev=eth_pcap0,iface=lo

stat:
	$(TMUXVSPLIT) zsh stat.sh


include $(prefix)/mk/vars.mk
LDFLAGS += -lslankdev -lpgen2

