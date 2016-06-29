

# include ../../mk/vars.mk
# include ../../mk/rules.mk

.SUFFIXES: .out .c .cc .o .h 
.cc.o: 
	$(CXX) $(CXXFLAGS) -c $< -o $@  $(LIBS)

DPDKPATH=$(RTE_SDK)/$(RTE_TARGET)

LIBS += \
	-Wl,--no-as-needed \
	-Wl,-export-dynamic \
	-L$(DPDKPATH)/lib \
	-lpthread \
	-pthread \
	-ldl \
	-lrt \
	-lpcap \
	-Wl,--whole-archive \
	-Wl,--start-group \
	-Wl,--end-group \
	-ldpdk \
	-Wl,--no-whole-archive

CXXFLAGS += -I$(DPDKPATH)/include -include $(DPDKPATH)/include/rte_config.h

SRC = dpdk.cc
OBJ = $(SRC:.cc=.o)


# $(CXX) $(CXXFLAGS) $(OBJS) $(LIBS) 

# def:
# 	$(CXX) $(CXXFLAGS) -c $(SRC) $(LIBS)


all: $(OBJ)
	@echo "BUILD DPDK's wrapper"
