

include ../mk/vars.mk


SRC = main.cc
OBJ = $(SRC:.cc=.o)


all: build-dataplane $(OBJ)
	$(CPP) $(OBJ) -o $(TARGET)


build-dataplane:
	$(MAKE) -C $(DATAPLANE_DIR)/dpdk


clean:
	$(RM) $(OBJ) $(TARGET)


