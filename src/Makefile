CC=g++
BOOST=/usr/local/include/boost-1_39/
V8=v8
CFLAGS= -m64 `mysql_config --cflags` -ggdb -I libev -I $(BOOST) -I$(V8)/include -Wall -DDEBUG -DEV_STANDALONE=1 -DEV_MULTIPLICITY=1
LDFLAGS=-m64 `mysql_config --libs` -ggdb -I libev -DDEBUG -lmysqlclient -lboost_thread-gcc41-mt  -lfcgi -lfcgi++ $(V8)/libv8_g.a
OBJS=acceptor.o worker.o processor.o  tracker.o sfcgi.o lbtt.o

all: tracker

tracker: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)
		

.cc.o:
	$(CC) $(CFLAGS) -c -O $*.cc

clean:
	rm -f *.o tracker
