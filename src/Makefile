


TARGET = a.out
SRCS   = main.cc
OBJS   = $(SRCS:.cc=.o)
include ../mk/vars.mk

all: build_lib $(TARGET)

build_lib:
	make -C lib

clean:
	$(RM) $(TARGET) $(OBJS)


include ../mk/runenv.mk

