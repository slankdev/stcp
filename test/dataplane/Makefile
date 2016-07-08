


prefix = ../..


TARGET = a.out
SRCS = main.cc \
	   $(prefix)/src/dataplane/dpdk.cc \
	   $(prefix)/src/dataplane/rte.cc
OBJS = $(SRCS:.cc=.o)
include $(prefix)/mk/vars.mk






all: $(TARGET)

wlan:
	sudo ./a.out  \
		--vdev=eth_pcap0,iface=wlxcce1d51517b5

enp:
	sudo ./a.out  \
		--vdev=eth_pcap1,iface=enp3s0

allif:
	sudo ./a.out \
		--vdev=eth_pcap0,iface=wlxcce1d51517b5 \
		--vdev=eth_pcap1,iface=enp3s0







