#include "worker.h"
#include <sys/types.h>
#include <sys/socket.h>


using namespace std;

void worker::spin(){
	asyncw.start();
	loop.loop();
}

void worker::add_session(int cfd){
	boost::mutex::scoped_lock lock( io_mutex );
	que.push(cfd);
	asyncw.send();
}

void worker::idle(ev::idle &w,int revents){
std::cout << "cleaning up sessions" << std::endl;
idlew.stop();
}

void worker::pqueue(ev::async &w,int revents){
boost::mutex::scoped_lock lock( io_mutex );
while(que.size() > 0){
        int s = que.front();
        que.pop();    
        ev::io *aio = new ev::io();
        aio->set<worker,&worker::process>(this);
        aio->set(loop);
        aio->start(s,ev::READ);
    }
}


void worker::closecleanup(ev::io &w){
	close(w.fd);
	w.stop();
	//cout << "deleting w" << endl;
	delete &w;
}

void worker::process(ev::io &w,int revents){
	if(revents & ev::TIMEOUT){
		closecleanup(w);return;
	}
	//we expect to receive the entire request in one packet
	//ignore if splitted
	int nread = recv(w.fd,m_read_b,sizeof(m_read_b),0);
	if(nread <= 0){
		closecleanup(w);
		return;	
	}
	//check for \r\n\r\n
		char *p;
		int i=4;
		p = strchr(m_read_b + nread - 1,'\n');
		while(p && (*p == '\r' || *p == '\n')){
			if(i < 0) break;
			i--;
			p--;
		}
		if(i != 0){
			closecleanup(w);return;
		}	
		
		//request is here, hand it off to processor
			int nwrite = m_processor.process(w.fd,m_read_b,nread,m_write_b,sizeof(m_write_b));
			send(w.fd,m_write_b,nwrite,0);
		closecleanup(w);
}

worker::worker(processor &h) : m_processor(h) {
	
	asyncw.set(loop);
	idlew.set(loop);
	idlew.set<worker,&worker::idle>(this);
	asyncw.set<worker,&worker::pqueue>(this);
}
