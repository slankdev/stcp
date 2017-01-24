

prefix = ../
TARGET = a.out
SRCS   = main.cc
OBJS   = $(SRCS:.cc=.o)
include $(prefix)/mk/vars.mk
CXXFLAGS += -Wno-format-security
LDFLAGS +=

DPDK_SRC = \
	dpdk/system.h \
	dpdk/mempool.h  \
	dpdk/port.h  \
	dpdk/cpu.h \
	dpdk/ring.h \
	dpdk/log.h \
	dpdk/struct_utils.h


THRD_SRC = \
	thread/txrx.h \
	thread/worker.h \
	thread/viewer.h \
	thread/omake.h \
	thread/rtc.h

FUNC_SRC =



all: $(TARGET)
main.o: main.cc  $(DPDK_SRC) $(THRD_SRC) $(FUNC_SRC)

clean:
	$(RM) $(TARGET) $(OBJS)

run: run_native

run_native:
	sudo ./a.out

run_dummy:
	sudo ./a.out   \
		--vdev=eth_null0 \
		--vdev=eth_null1

run_vagrant:
	sudo ./a.out   \
		--vdev=eth_pcap0,iface=vboxnet2 \
		--vdev=eth_pcap1,iface=lo


gdb:
	sudo gdb \
		--args a.out

		# --vdev=eth_pcap0,iface=vboxnet2 \
		# --vdev=eth_pcap1,iface=vboxnet3
		# --args a.out \
		# --args a.out
		# --vdev=eth_null0  \
		# --vdev=eth_null1

# run:
# 	sudo ./a.out \
# 		--vdev=eth_pcap0,iface=lo
#
# gdb:
# 	sudo gdb --args a.out \
# 		--vdev=eth_pcap0,iface=lo \
# 		--vdev=eth_null3
