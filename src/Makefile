

prefix = ..

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
	sudo ./$(TARGET) \
		--vdev=eth_pcap0,iface=eth0

runenp:
	sudo ./$(TARGET) \
		--vdev=eth_pcap0,iface=enp0s20u2

runlo:
	sudo ./$(TARGET) \
		--vdev=eth_pcap0,iface=lo

runall:
	sudo ./$(TARGET) \
		--vdev=eth_pcap0,iface=eth0 \
		--vdev=eth_pcap1,iface=eth1 \
		--vdev=eth_pcap2,iface=lo 

gdb:
	sudo gdb -q --args \
		./$(TARGET) --vdev=eth_pcap0,iface=eth0

gdblo:
	sudo gdb -q --args \
		./$(TARGET) --vdev=eth_pcap0,iface=lo

include $(prefix)/mk/vars.mk
LDFLAGS += -lslankdev -lpgen2

