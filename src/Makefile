

prefix = ..

TARGET = a.out
SRCS   = main.cc \
		 net_device.cc \
		 rte.cc \
		 dpdk.cc \
		 ifaddr.cc \
		 stcp.cc
		 


OBJS   = $(SRCS:.cc=.o)



all: $(TARGET)

clean:
	$(RM) $(TARGET) $(OBJS) stcp.log



run:
	@sudo rm -rf stcp.log
	sudo ./$(TARGET) \
		--vdev=eth_pcap0,iface=lo
	@cat stcp.log

runeth0:
	sudo ./$(TARGET) \
		--vdev=eth_pcap0,iface=eth0

runall:
	sudo ./$(TARGET) \
		--vdev=eth_pcap0,iface=eth0 \
		--vdev=eth_pcap1,iface=eth1 \
		--vdev=eth_pcap2,iface=lo 



include $(prefix)/mk/vars.mk
LDFLAGS += -lslankdev

