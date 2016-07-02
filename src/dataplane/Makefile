




.SUFFIXES: .out .c .cc .o .h 
.cc.o: 
	@echo "[CXX] $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@  $(LIBS)


LIBS += \
	-Wl,--no-as-needed \
	-Wl,-export-dynamic \
	-L$(RTE_SDK)/$(RTE_TARGET)/lib \
	-pthread \
	-lpthread -ldl -lrt -lpcap \
	-Wl,--whole-archive \
	-Wl,--start-group \
	-ldpdk \
	-Wl,--end-group \
	-Wl,--no-whole-archive

CXXFLAGS = -Wall -Wextra
CXXFLAGS += -std=c++11 -I$(RTE_SDK)/$(RTE_TARGET)/include \
			-include $(RTE_SDK)/$(RTE_TARGET)/include/rte_config.h


TARGET = a.out
SRC = main.cc dpdk.cc
OBJ = $(SRC:.cc=.o)


all: $(OBJ)
	@echo "link"
	@$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ) $(LIBS)

clean:
	$(RM) $(OBJ) $(TARGET)


run:
	sudo ./a.out --vdev=eth_pcap0,iface=wlan0
