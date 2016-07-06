

# Test codes for each-modules

## Usage

```
$ cd MODDIR
$ make
$ make run
```

## Template Makefile


```
prefix = $(STCP_FULLPATH)
TARGET = $(TARGETNAME)
SRCS   = $(SOURCEFILES)
OBJS   = $(SRCS:.cc=.o)

all: $(TARGET)

include $(prefix)/mk/vars.mk
```
