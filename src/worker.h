/* 
 * File:   workers.h
 * Author: lfontes
 *
 * Created on September 4, 2009, 5:12 PM
 */

#ifndef _WORKERS_H
#define	_WORKERS_H
#include "tracker.h"
#include <pthread.h>
#include <queue>
#include <ev++.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>

using namespace std;

class Worker {
public:
    void newClient(int fd);

    void operator()() {
        loop.loop();
    }
    Worker(TorrentTracker &t);
    void process(int fd);
    ev::dynamic_loop loop; //our main loop
    int timeout;
private:
    int decode_q(const char *uri, char *ret);
    void sendError(int fd);
    void sendit(int fd);
    void sendError(int fd, char *msg);
    bool prepare(TorrentRequest &req, char *path, bool validate);
    char inB[4096];
    int nread;
    ev::async asyncw;
    TorrentTracker &tracker;
    void receiveClient(ev::async &w, int revents);
    pthread_mutex_t io_mutex;
    queue<int> clients;
    stringstream output;
};

struct WorkerClient {
    WorkerClient(int f, int timeout, Worker & w);
    ev::io iow;
    ev::timer timerw;
    Worker &worker;
    void timeout(ev::timer &t, int revents);
    void receive(ev::io &iw, int revents);
};


#endif	/* _WORKERS_H */

