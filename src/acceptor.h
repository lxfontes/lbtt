#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__

#include <vector>
#include <iostream>
#include <fcntl.h>
#include <ev++.h>
#include "worker.h"
#include "processor.h"
#include "thread.hpp"
class acceptor{
public:
	acceptor(const char *ipp,int p,int t,processor &h);
	void run();
protected:
	void newcon(ev::io &,int);
	vector<worker *> workers;
	vector<thread<worker> *> threads;
	ev::dynamic_loop loop; //our main loop
	ev::io iow;
	int fd;
	int port;
	const char *ip;
	int nthread; //max threads
	int athread; //actual thread
};
#endif
