CC=g++
OBJS=main.o tracker.o acceptor.o worker.o v8funcs.o
V8=../deps/v8
CFLAGS=-Wall -I ../deps/libev  -I$(V8)/include -DEV_STANDALONE=1 -DEV_MULTIPLICITY=1 `mysql_config --cflags`
LDFLAGS=-lpthread  $(V8)/libv8.a `mysql_config --libs`


UNAME := $(shell uname)
ifeq ($(UNAME), FreeBSD)
CFLAGS+= -I/usr/local/include
LDFLAGS+= -lexecinfo -L/usr/local/lib
endif


PLATFORM := $(shell uname -m)
ifeq ($(PLATFORM), x86_64)
V8ARCH=x64
else
ifeq ($(PLATFORM), amd64)
V8ARCH=x64
else
V8ARCH=ia32
endif
endif

ifeq ($(DEBUG), 1)
CFLAGS += -ggdb -DDEBUG
endif


all: v8 tracker

tracker: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

v8: $(V8)/libv8.a
	cd $(V8) && scons mode=release library=static arch=$(V8ARCH)

.cpp.o:
	$(CC) $(CFLAGS) -c -O $*.cpp

clean:
	rm -f *.o tracker
