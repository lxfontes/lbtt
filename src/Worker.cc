#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "Worker.h"
#include "FastCgi.h"


using namespace std;

void Worker::run(){
	asyncw.start();
	loop.loop();
}

void Worker::add_session(int cfd){
	scoped_lock lock( io_mutex );
	que.push(cfd);
	asyncw.send();
}

void Worker::pqueue(ev::async &w,int revents){
	scoped_lock lock( io_mutex );
	while(que.size() > 0){
		int s = que.front();
		que.pop();
		ev::io *aio = new ev::io();
		aio->set<Worker,&Worker::process>(this);
		aio->set(loop);
		aio->start(s,ev::READ);
	}
}

bool Worker::setError(const char *x){
	m_output << "HTTP/1.0 400 Bad Request\r\nContent-Type: text/html\r\n\r\n" << x << "\r\n";
	return true; 
}

bool Worker::processRequest(ev::io &w){
	char *inb = m_read_b;

	bool scrape = false;
	//easier to play with plain c char
	if(strncmp("GET",inb,3) != 0){
		cout << inb << endl;
		return setError("request is not a GET");
	}
	char *query = NULL;
	char *proto = NULL;
	//process first line
	query = strchr(inb,' ');
	if(query == NULL || ((proto=strchr(query + 1,' ')) == NULL)){

		return setError("request is not a GET");
	}
	*proto = '\0';
	query++;

	//check path
	if(strncmp("/scrape",query,7) == 0){
		scrape = true;
	}else if(strncmp("/announce",query,9) != 0){
		return setError("neither scrape nor announce");
	}
	
	req.init();
	if(strchr(query,'?') == NULL){
		return setError("No query string");
	}
	char *p,*argument = query;
	char bvalue[20];
	int nvalue;
	strsep(&argument,"?");
	p = argument;

	if(p == NULL || *p == '\0')return setError("no query string");

	struct sockaddr_in mysock;
	socklen_t namelen;
	namelen = sizeof(struct sockaddr_in);
	getpeername(w.fd, (struct sockaddr *)&mysock, &namelen);
	req.ip = mysock.sin_addr.s_addr;

	while (p != NULL && *p != '\0') {
		char *key, *value;
		argument = strsep(&p, "&");

		value = argument;
		key = strsep(&value, "=");
		if (value == NULL)return setError("error parsing query string");
		nvalue = FastCgi::decode_q(value,bvalue);

		if(strcmp("info_hash",key) == 0){
			//check length
			if(nvalue != 20)return setError("error parsing query string(hash)");
			memcpy(req.torrent,bvalue,20);
			req.valid++;
		}else if(strcmp("peer_id",key) == 0){
			if(nvalue != 20)return setError("error parsing query string(peer)");
			memcpy(req.peerid,bvalue,20);
			req.valid++;
		}else if(strcmp("left",key) == 0){
			req.left = atol(bvalue);
			req.valid++;
		}else if(strcmp("passkey",key) == 0){
			strncpy(req.pass,value,20); //don't use hex encoded in passkeys !
		}else if(strcmp("corrupt",key) == 0){
			req.corrupt = atol(bvalue);
			req.valid++;
		}else if(strcmp("downloaded",key) == 0){
			req.download = atol(bvalue);
			req.valid++;
		}else if(strcmp("uploaded",key) == 0){
			req.upload = atol(bvalue);
			req.valid++;
		}else if(strcmp("event",key) == 0){
			if(strcmp(bvalue,"started")==0){
				req.event = req.START;
			}else if(strcmp(bvalue,"stopped")==0){
				req.event = req.STOP;
			}else if(strcmp(bvalue,"completed")==0){
				req.event = req.COMPLETE;
			}
			req.valid++;
		}else if(strcmp("port",key)==0){
			req.port = atoi(bvalue);
			req.valid++;
		}else if(strcmp("numwant",key) ==0){
			req.numwant = atoi(bvalue);
			if(req.numwant > 30 || req.numwant < 1)
			req.numwant = 30;
			req.valid++;
		}else if(strcmp("compact",key) == 0){
			req.compact = true;
		}else if(strcmp("ip",key)==0){
			req.ip = inet_addr(bvalue);
			if(req.ip == INADDR_NONE)return setError("error parsing query string(ip)");
		}
	}

	if(req.valid < 7 && scrape == false){
		return setError("invalid request");
	}
	//put the http header in there
	m_output << "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n";
	if(scrape)
	m_tracker.scrape(req,m_output);
	else
	m_tracker.announce(req,m_output);
	return true;
}


void Worker::process(ev::io &w,int revents){
	if(revents & ev::TIMEOUT){
		closecleanup(w);return;
	}
	//we expect to receive the entire request in one packet
	//ignore if splitted
	nread = recv(w.fd,m_read_b,sizeof(m_read_b),0);
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
	m_output.clear();
	m_output.str("");
	//yep, we have a request
	processRequest(w);
	string toSend = m_output.str();
	send(w.fd,toSend.c_str(),toSend.length(),0);
	closecleanup(w);
}




void Worker::closecleanup(ev::io &w){
	close(w.fd);
	w.stop();
	//cout << "deleting w" << endl;
	delete &w;
}


Worker::Worker(Tracker &t):m_tracker(t){
	pthread_mutex_init(&io_mutex,NULL);
	asyncw.set(loop);
	idlew.set(loop);
	//        idlew.set<Worker,&Worker::idle>(this);
	asyncw.set<Worker,&Worker::pqueue>(this);
}
