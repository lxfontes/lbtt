/* 
 * File:   request.h
 * Author: lfontes
 *
 * Created on September 4, 2009, 4:00 PM
 */

#ifndef _REQUEST_H
#define	_REQUEST_H
#include <netinet/in.h>
#include <map>
#include <vector>
#include <iostream>


#include <unistd.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

struct hashCmp {

    bool operator()(const char *s1, const char *s2) const {
        return memcmp(s1, s2, 20) < 0;
    }
};


struct TorrentRequest {

    enum Tevent {
        UPDATE, COMPLETE, START, STOP
    };
    Tevent event;

    unsigned long left;
    unsigned long corrupt;
    unsigned long download;
    unsigned long upload;
    bool compact;
    char peerid[20];
    char torrent[20];
    char pass[32];
    unsigned int port;
    in_addr_t ip;
    unsigned int numwant;
    void printMe(){
        cout << "req left " << left << " corrupt " << corrupt << " download " << download << " upload " << upload << " port "<< port <<endl;
    }
    TorrentRequest() :
    left(0), corrupt(0), download(0), upload(0), compact(0), port(0), ip(0), numwant(30) {
        peerid[0] = torrent[0] = pass[0] = '\0';
    }
};

struct TorrentPeer {
    char id[20];
    unsigned long left;
    unsigned long corrupt;
    unsigned long download;
    unsigned long upload;
    unsigned int port;
    char *pass;
    double lastseen;
    in_addr_t ip;
	
 enum Pstate{
 LEECH,SEED };
   Pstate state;
    TorrentPeer(char *s) :
    left(0), corrupt(0), download(0), upload(0), port(0), ip(0),state(LEECH) {
        memcpy(id, s, 20);
    }
};



struct TorrentFile {
    char id[20];
    unsigned long hosts;
    unsigned long seeders;
    unsigned long downloads;
    double lastseen;
    map<const char *, TorrentPeer *,hashCmp> peers;
    vector<TorrentPeer *> peerCache;
    TorrentFile(char *s):
    hosts(0),seeders(0),downloads(0),lastseen(0){
        memcpy(id,s,20);
    }
};
#endif	/* _REQUEST_H */

