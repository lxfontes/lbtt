CC=g++
V8=../deps/v8
CFLAGS=`mysql_config --cflags` -ggdb -I ../deps/libev  -I$(V8)/include -Wall -DDEBUG -DEV_STANDALONE=1 -DEV_MULTIPLICITY=1
LDFLAGS=`mysql_config --libs` -ggdb -I ../deps/libev -DDEBUG -lmysqlclient -lpthread  -lfcgi -lfcgi++ $(V8)/libv8_g.a
OBJS=acceptor.o worker.o processor.o  tracker.o sfcgi.o lbtt.o

all: tracker

tracker: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)
		

.cc.o:
	$(CC) $(CFLAGS) -c -O $*.cc

clean:
	rm -f *.o tracker
