
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>


#include "acceptor.h"
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


void acceptor::newcon(ev::io &w,int revents){

        int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
        client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd == -1) {
                return;
        }

        if (setnonblock(client_fd) < 0)
                exit(1);

 

 		workers[athread++]->add_session(client_fd);
        if(athread >= nthread) athread = 0;


}



void acceptor::run(){
  struct sockaddr_in listen_addr;
    int reuseaddr_on = 1;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_on,
            sizeof(reuseaddr_on)) == -1)
            exit(1);
    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = inet_addr(ip);
    listen_addr.sin_port = htons(port);
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



acceptor::acceptor(const char *pp,int p,int t,processor &h):m_processor(h) {
		ip = pp;
        iow.set(loop);
        iow.set<acceptor,&acceptor::newcon>(this);
        athread = 0;
        thread<worker> *thr;
        nthread = t>50?50:t;
        port = p;
        
        
        for (int i = 0; i < nthread; ++i){
                worker *w = new worker(m_processor);
                workers.push_back(w);
                thr = new thread<worker>(w);
        		threads.push_back(thr);
        }
}
