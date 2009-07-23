#ifndef __WORKER_H__
#define __WORKER_H__

#include <iostream>
#include <fcntl.h>
#include <ev++.h>
#include <queue>
#include "scoped_lock.hpp"
#include "processor.h"
	
class worker{
public:
	worker(processor &h);
	void run();
	void add_session(int cfd);
	void idle(ev::idle &,int);
	void pqueue(ev::async &,int);
	void process(ev::io &,int);
protected:
	void closecleanup(ev::io &);
	std::queue<int> que;	
	ev::async asyncw;
	ev::idle idlew;
	ev::dynamic_loop loop;
	pthread_mutex_t io_mutex ;
	processor m_processor;
	char m_read_b[4 << 10]; //thanks xbtt :)
	char m_write_b[4 << 10]; //thanks xbtt :)
};

#endif
