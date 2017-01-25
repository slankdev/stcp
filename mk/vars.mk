
ifeq ($(prefix),)
$(error prefix is not defined)
endif

include $(prefix)/mk/commands.mk
include $(prefix)/mk/rules.mk

INCLUDES   += \
			 -I$(prefix)/src \
			 -I$(prefix)/src/include \
			 -I$(RTE_SDK)/$(RTE_TARGET)/include \
			 -I/home/slank/git/libslankdev/include \
			 -include $(RTE_SDK)/$(RTE_TARGET)/include/rte_config.h

LDFLAGS = $(DPDK_LDFLAGS)

DPDK_LDFLAGS += \
	-Wl,--no-as-needed \
	-Wl,-export-dynamic \
	-L$(RTE_SDK)/$(RTE_TARGET)/lib \
	-lpthread -ldl -lrt -lm -lpcap \
	-Wl,--whole-archive \
	-Wl,--start-group \
	-ldpdk \
	-Wl,--end-group \
	-Wl,--no-whole-archive

MAKE      = make
CXX       = g++

CXXFLAGS += -Wall -Wextra
ifeq ($(CXX), clang++)
CXXFLAGS += -Weverything # Thankyou, @herumi.
endif
CXXFLAGS += -std=c++11
CXXFLAGS += -m64 -pthread -march=native $(INCLUDES)







