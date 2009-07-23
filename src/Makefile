CC=g++
V8=../deps/v8

CFLAGS=`mysql_config --cflags` -I ../deps/libev  -I$(V8)/include -Wall -DDEBUG -DEV_STANDALONE=1 -DEV_MULTIPLICITY=1
LDFLAGS=`mysql_config --libs` -I ../deps/libev -DDEBUG -lmysqlclient -lpthread  -lfcgi -lfcgi++ $(V8)/libv8.a

UNAME := $(shell uname)
ifeq ($(UNAME), FreeBSD)
CFLAGS+=-I/usr/local/include
LDFLAGS+=-lexecinfo -L/usr/local/lib
endif

PLATFORM := $(shell uname -m)
ifeq ($(PLATFORM), x86_64)
V8ARCH=x64
else
V8ARCH=ia32
endif

ifeq ($(DEBUG), 1)
CFLAGS += -ggdb -DDEBUG
endif



OBJS=acceptor.o worker.o processor.o  tracker.o sfcgi.o lbtt.o

all: v8 tracker
v8:
	cd ../deps/v8 && scons mode=release library=static snapshot=on arch=$(V8ARCH)

tracker: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)
		

.cc.o:
	$(CC) $(CFLAGS) -c -O $*.cc

clean:
	rm -f *.o tracker
