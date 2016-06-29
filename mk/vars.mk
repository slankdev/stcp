


prefix = /home/slank/git/stcp

include $(CURDIR)/mk/commands.mk
include $(CURDIR)/mk/rules.mk


CPP   = g++
CPPFLAGS = -Wextra

MAKE       = make


LIB          = -L$(DPDK_LIB_DIR) -ldpdk
INCLUDE      = -I$(INCLUDE_DIR) \
			   -I$(DPDK_INCLUDE_DIR)

SRC_DIR = src
TARGET  = stcp





# DIR paths
DATAPLANE_DIR = $(prefix)/src/dataplane
INSTALL_DIR   = $(prefix)/src/include
INCLUDE_DIR   = $(prefix)/src/include

DPDK_LIB_DIR     = $(RTE_SDK)/$(RTE_TARGET)/lib 
DPDK_INCLUDE_DIR = $(RTE_SDK)/$(RTE_TARGET)/include


