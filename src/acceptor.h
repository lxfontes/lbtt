#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__

#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/thread/thread.hpp>
#include <iostream>
#include <fcntl.h>
#include <ev++.h>
#include "worker.h"
#include "processor.h"

class acceptor{
public:
	acceptor(const char *ipp,int p,int t,processor &h);
	void run();
protected:
	void newcon(ev::io &,int);
	boost::array<worker *,50> workers;
	boost::thread_group threads;
	ev::dynamic_loop loop; //our main loop
	ev::io iow;
	int fd;
	int port;
	const char *ip;
	int nthread; //max threads
	int athread; //actual thread
	processor m_processor;
};
#endif
