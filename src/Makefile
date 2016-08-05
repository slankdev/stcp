

prefix = ..

TARGET = a.out
LOGNAME = log.log
SRCS   = \
		main.cc \
		ifaddr.cc \
		dpdk.cc \
		ifnet.cc \
		rte.cc \
		mbuf.cc \
		arp.cc \
		stcp.cc


OBJS   = $(SRCS:.cc=.o)


all: $(TARGET)

clean:
	$(RM) $(TARGET) $(OBJS) $(LOGNAME)

re: clean all




run:
	sudo ./$(TARGET) \
		--vdev=eth_pcap0,iface=eth0

runlo:
	sudo ./$(TARGET) \
		--vdev=eth_pcap0,iface=lo

runall:
	sudo ./$(TARGET) \
		--vdev=eth_pcap0,iface=eth0 \
		--vdev=eth_pcap1,iface=eth1 \
		--vdev=eth_pcap2,iface=lo 


include $(prefix)/mk/vars.mk
LDFLAGS += -lslankdev -lpgen2

