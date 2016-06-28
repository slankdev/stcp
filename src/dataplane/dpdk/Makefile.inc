
DATAPATHSRCS += dpdk.cc
LDFLAGS += -lpcap -L$(RTE_LIBDIR) -ldpdk
