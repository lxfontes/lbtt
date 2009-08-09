#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "Acceptor.h"
#include "thread.hpp"


using namespace std;


int
setnonblock(int fd)
{
	int flags;

	flags = fcntl(fd, F_GETFL);
	if (flags < 0)
	return flags;
	flags |= O_NONBLOCK;
	if (fcntl(fd, F_SETFL, flags) < 0)
	return -1;

	return 0;
}


void Acceptor::newcon(ev::io &w,int revents){

	int client_fd;
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
	if (client_fd == -1) {
		return;
	}
	if (setnonblock(client_fd) < 0)
	exit(1);
	//push work to thread ( simplest round robin ever)
	workers[m_actualThread++]->add_session(client_fd);
	if(m_actualThread >= m_threads) m_actualThread = 0;
}


void Acceptor::run(){
	struct sockaddr_in listen_addr;
	int reuseaddr_on = 1;
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on,
				sizeof(reuseaddr_on)) == -1)
	exit(1);
	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = inet_addr(m_ip.c_str());
	listen_addr.sin_port = htons(m_port);
	if (bind(fd, (struct sockaddr *)&listen_addr,
				sizeof(listen_addr)) < 0)
	exit(1);
	if (listen(fd,5) < 0)
	exit(1);
	if (setnonblock(fd) < 0)
	exit(1);


	iow.start(fd,ev::READ);

	loop.loop();
}



Acceptor::Acceptor(Tracker &t,string &ip,int port,int threads):
m_port(port),m_ip(ip),m_tracker(t)
{
	iow.set(loop);
	iow.set<Acceptor,&Acceptor::newcon>(this);
	m_actualThread = 0;
	thread<Worker> *thr;
	m_threads = threads;

	for (int i = 0; i < m_threads; ++i){
		Worker *w = new Worker(m_tracker);
		workers.push_back(w);
		thr = new thread<Worker>(w);
		m_threadpool.push_back(thr);
	}
}
