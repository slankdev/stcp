

prefix = ..

# INTERFACE  = eth0
# INTERFACE  = wlp3s0
# INTERFACE  = enp0s20u2
INTERFACE  = eth1
# INTERFACE  = lo
# INTERFACE = vboxnet1
# INTERFACE = vboxnet0

TMUXSPLIT  = tmux split-window -v
TMUXVSPLIT = tmux split-window -h
TARGET = a.out
LOGNAME = *.log
SRCS   = \
		ifnet.cc \
		ncurses.cc \
		dataplane.cc \
		protos/ethernet.cc \
		protos/arp.cc \
		protos/ip.cc \
		protos/icmp.cc \
		protos/udp.cc \
		protos/tcp.cc \
		protos/tcp_socket.cc \
		stcp.cc \
		debug.cc \
		main.cc


OBJS   = $(SRCS:.cc=.o)


all: $(TARGET)

clean:
	$(RM) $(TARGET) $(OBJS) $(LOGNAME)

re: clean all


runc:
	sudo ./$(TARGET) \
		--vdev=eth_pcap0,iface=$(INTERFACE)

runall:
	$(TMUXSPLIT) \
	sudo ./$(TARGET) \
		--vdev=eth_pcap0,iface=eth0 \
		--vdev=eth_pcap1,iface=eth1 \
		--vdev=eth_pcap2,iface=lo

perf:
	sudo perf record ./$(TARGET) \
		--vdev=eth_pcap0,iface=$(INTERFACE)

perfrep:
	sudo perf report


gdb:
	sudo gdb -q --args \
		./$(TARGET) --vdev=eth_pcap0,iface=$(INTERFACE)


include $(prefix)/mk/vars.mk
LDFLAGS += -lncurses

