#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__

#include <vector>
#include <iostream>
#include <fcntl.h>
#include <ev++.h>
#include "Worker.h"
#include "Tracker.h"
#include "thread.hpp"
using namespace std;

class Acceptor{
public:
	Acceptor(Tracker &t,string &ip,int port,int threads);
	void run();
protected:
	void newcon(ev::io &,int);
	vector<Worker *> workers;
	vector<thread<Worker> *> m_threadpool;
	ev::dynamic_loop loop; //our main loop
	ev::io iow;
	int fd;
	int m_port;
	string m_ip;
	int m_threads; //max threads
	int m_actualThread; //actual thread
	Tracker &m_tracker;
};
#endif
