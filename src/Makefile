


TARGET = a.out
SRCS   = main.cc
OBJS   = $(SRCS:.cc=.o)
include ../mk/vars.mk

all: $(TARGET)

lib:
	make -C ../lib

libclean:
	make -C ../lib clean

re:
	make clean
	make libclean
	make lib
	make

clean:
	$(RM) $(TARGET) $(OBJS)

include ../mk/runenv.mk

