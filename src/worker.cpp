#include <queue>
#include <iostream>
#include <sstream>
#include "worker.h"
#include "scoped_lock.h"

using namespace std;

Worker::Worker(TorrentTracker& t) :
timeout(30),
tracker(t) {
    pthread_mutex_init(&io_mutex, NULL);
    asyncw.set(loop);
    asyncw.set<Worker, &Worker::receiveClient > (this);
    asyncw.start();
}

void Worker::newClient(int fd) {
    scoped_lock lock(io_mutex);
    clients.push(fd);
    asyncw.send();
}

void Worker::receiveClient(ev::async& w, int revents) {
    scoped_lock lock(io_mutex);
    WorkerClient *client;
    while (clients.size() > 0) {
        int fd = clients.front();
        clients.pop();
        client = new WorkerClient(fd, timeout, *this);
    }
}

void Worker::sendit(int fd) {
    string tosend = output.str();
    send(fd, tosend.c_str(), tosend.length(),0);
}

void Worker::sendError(int fd, char *msg) {
    output << "HTTP/1.0 500 HTTP Error\r\n\r\n" << msg << endl;
    string tosend = output.str();
    sendit(fd);
}

bool Worker::prepare(TorrentRequest& req, char* path, bool validate) {
    //info_hash is REQUIRED
    char *p = NULL;
    char *argument = path;
    int hits = 0;
    bool ret = false;
    //cout << "query " << path << endl;
    strsep(&argument, "?");
    p = argument;
    if (p == NULL || *p == '\0')return false;
    while (p != NULL && *p != '\0') {
        char *key, *value;
        argument = strsep(&p, "&");
        value = argument;
        key = strsep(&value, "=");
        if (value == NULL) return false;

        if (strcmp("info_hash", key) == 0) {
            if (decode_q(value, req.torrent) != 20) return false;
            ret = true;
            hits++;
        } else if (strcmp("peer_id", key) == 0) {
            if (decode_q(value, req.peerid) != 20) return false;
            hits++;
        } else if (strcmp("left", key) == 0) {
            req.left = atol(value);
            hits++;
        } else if (strcmp("passkey", key) == 0) {
            strncpy(req.pass, value, 20); //don't use hex encoded in passkeys !
        } else if (strcmp("corrupt", key) == 0) {
            req.corrupt = atol(value);
            hits++;
        } else if (strcmp("downloaded", key) == 0) {
            req.download = atol(value);
            hits++;
        } else if (strcmp("uploaded", key) == 0) {
            req.upload = atol(value);
            hits++;
        } else if (strcmp("event", key) == 0) {
            if (strcmp(value, "started") == 0) {
                req.event = req.START;
            } else if (strcmp(value, "stopped") == 0) {
                req.event = req.STOP;
            } else if (strcmp(value, "completed") == 0) {
                req.event = req.COMPLETE;
            }
            hits++;
        } else if (strcmp("port", key) == 0) {
            req.port = atoi(value);
            hits++;
        } else if (strcmp("numwant", key) == 0) {
            req.numwant = atoi(value);
            if (req.numwant > 30 || req.numwant < 1)
                req.numwant = 30;
            hits++;
        } else if (strcmp("compact", key) == 0) {
            req.compact = true;
        } else if (strcmp("ip", key) == 0) {
            req.ip = inet_addr(value);
            if (req.ip == INADDR_NONE)return false;
        }

    }
    if(validate && hits < 7)
        return false;

    return ret;

}

void Worker::process(int fd) {
    nread = recv(fd, inB, sizeof (inB), 0);
    if (nread == 0) return;
    inB[nread - 1] = '\0';
    output.clear();
    output.str("");
    char *path = NULL;
    char *p = NULL;
    if (strncmp("GET ", inB, 4) != 0) {
        sendError(fd,"invalid request");
        return;
    }
    path = &inB[4];
    p = strchr(path, ' ');
    if (p == NULL || memcmp(p, " HTTP", 5) != 0) {
        sendError(fd,"invalid request");
        return;
    }
    *p = '\0';



    output << "HTTP/1.0 200 HTTP OK\r\nContent-Type: text/html\r\n\r\n";
    //doesnt require infohash
    //status
    if (strncmp("/lbtt/status", path, 12) == 0) {
        tracker.status(output);
        sendit(fd);
        return;
    }

    TorrentRequest req;
    struct sockaddr_in mysock;
    socklen_t namelen;
    namelen = sizeof (struct sockaddr_in);
    getpeername(fd, (struct sockaddr *) & mysock, &namelen);
    req.ip = mysock.sin_addr.s_addr;

    //requires infohash
    // announce , scrape , torrentinfo
    
    if ((strncmp("/announce?", path, 8) == 0) && (prepare(req, path, false) == true)) {
        tracker.announce(req, output);
    } else if ((strncmp("/scrape?", path, 8) == 0) && (prepare(req, path, false) == true)) {
        tracker.scrape(req, output);
    } else if ((strncmp("/lbtt/info?", path, 8) == 0) && (prepare(req, path, false) == true)) {
        tracker.info(req, output);
    } else {
        output << "how did you get here?" << endl;
    }
    sendit(fd);
}

int Worker::decode_q(const char *uri, char *ret) {
    char c;
    int i, j, in_query = 0;

    for (i = j = 0; uri[i] != '\0' && uri[i] != '&'; i++) {
        c = uri[i];
        if (j >= 20) return 99; //parser error
        if (c == '?') {
            in_query = 1;
        } else if (c == '+' && in_query) {
            c = ' ';
        } else if (c == '%' && isxdigit(uri[i + 1]) &&
                isxdigit(uri[i + 2])) {
            char tmp[] = {uri[i + 1], uri[i + 2], '\0'};
            c = (char) strtol(tmp, NULL, 16);
            i += 2;
        }

        ret[j++] = c;
    }

    return j;
}

/* -------------------------- */
WorkerClient::WorkerClient(int f, int timeout, Worker & w) :
worker(w) {
    iow.set(w.loop);
    iow.set<WorkerClient, &WorkerClient::receive > (this);
    iow.start(f, ev::READ);
    timerw.set(w.loop);
    timerw.set<WorkerClient, &WorkerClient::timeout > (this);
    timerw.start(timeout, 0.);

}

void WorkerClient::timeout(ev::timer& t, int revents) {
    close(iow.fd);
    timerw.stop();
    iow.stop();
    cout << "Client timed out" << endl;
    delete this;
}

void WorkerClient::receive(ev::io& iw, int revents) {
    worker.process(iow.fd);
    timerw.stop();
    iow.stop();
    close(iow.fd);
    delete this;
}

