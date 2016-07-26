

prefix = ..

TARGET = a.out
SRCS = rte.cc \
	   dpdk.cc \
	   main.cc \

OBJS = $(SRCS:.cc=.o)



all: $(TARGET)

clean:
	$(RM) $(TARGET) $(OBJS)





run:
	sudo ./$(TARGET) \
		--vdev=eth_pcap0,iface=eth0 \
		--vdev=eth_pcap1,iface=lo 

runall:
	sudo ./$(TARGET) \
		--vdev=eth_pcap0,iface=eth0 \
		--vdev=eth_pcap1,iface=wlxcce1d51517b5 \
		--vdev=eth_pcap2,iface=lo 


include $(prefix)/mk/vars.mk
LDFLAGS += -lslankdev

