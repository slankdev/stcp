


prefix = ../..

TARGET = a.out

SRCS   = main.cc \
		 $(prefix)/src/protocol/protocol.cc \
		 $(prefix)/src/dataplane/dpdk.cc \
		 $(prefix)/src/dataplane/rte.cc

OBJS   = $(SRCS:.cc=.o)


all: $(TARGET)



run: all
	sudo ./$(TARGET) \
	--vdev=eth_pcap0,iface=enp3s0 


runall: all
	sudo ./$(TARGET) \
	--vdev=eth_pcap0,iface=enp3s0 \
	--vdev=eth_pcap1,iface=wlxcce1d51517b5


include $(prefix)/mk/vars.mk
clean:
	$(RM) $(TARGET) main.o
LDFLAGS += -lpgen2
