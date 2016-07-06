

prefix = ..

TARGET = a.out
SRCS = main.cc \
	   dataplane/dpdk.cc \
	   dataplane/rte.cc \

OBJS = $(SRCS:.cc=.o)



all: $(TARGET)

runall:
	sudo ./$(TARGET) \
		--vdev=eth_pcap0,iface=enp3s0 \
		--vdev=eth_pcap1,iface=wlxcce1d51517b5


runwlan:
	sudo ./$(TARGET) \
		--vdev=eth_pcap1,iface=wlxcce1d51517b5

runenp:
	sudo ./$(TARGET) \
		--vdev=eth_pcap0,iface=enp3s0 


include $(prefix)/mk/vars.mk

