#ifndef __TRACKER_H__
#define __TRACKER_H__
#include <ostream>
#include <sstream>
#include <map>
#include <netinet/in.h>
#include <v8.h>
#include "thread.hpp"
using namespace std;
using namespace v8;
//hash compare functor for peer_id and infohash
struct hashCmp {
      bool operator()( const char *s1, const char *s2 ) const {
	    //cout << "comparing " << s1 << " with " << s2 << endl;
        return memcmp( s1, s2,20 ) < 0;
      }
    };

struct Request{
        enum mevent{
                UPDATE,COMPLETE,START,STOP
        };
        mevent event;
        unsigned long left;
        unsigned long corrupt;
        unsigned long download;
        unsigned long upload;
        bool compact;
	char peerid[20];
	char torrent[20];
	char pass[20];
        int port;
        in_addr_t ip;
        unsigned int numwant;
        int valid;
        void init(){torrent[0] = peerid[0] = pass[0]='\0';corrupt=download=upload=left=valid=port=ip=0;event = UPDATE;compact=false;}
};

    
struct Peer{
	Peer(char *s){memcpy(id,s,20);left = ip = port = corrupt = download = upload = 0;}
	char id[20];
	unsigned long left;
	unsigned long corrupt;
	unsigned long download;
	unsigned long upload;
	int port;
	in_addr_t ip;
	string pass;
	double lastseen;
};

struct Torrent{
	Torrent(char *s):peers() {memcpy(id,s,20); hosts = seeders = download = 0;}
	char id[20];
	map<const char *,Peer *, hashCmp> peers;
	unsigned long hosts;
	unsigned long seeders;
	unsigned long download;
	double lastseen;
};


class Tracker{

public:
void scrape(Request &req,stringstream &output);
void announce(Request &req,stringstream &output);
void run();
Tracker(string &);
        void status(ostream &);
        void info(ostream &,char *);

int interval;
int cleanupInterval;
int expireTimeout;

protected:
map<const char *,Torrent *, hashCmp> torrents;
void removePeer(Torrent *,Peer *);
void removeTorrent(Torrent *);
void peerList(Request &,Torrent *,stringstream &);
void setError(stringstream &,const char *);
unsigned long seeders;
unsigned long hosts;
unsigned long download;
pthread_mutex_t io_mutex ;
time_t startTime;

        //v8 specific
        Handle<Context> context;
        Persistent<ObjectTemplate> global;

        Persistent<Function> newRequest;
        Persistent<Function> expirePeer;
        Persistent<Function> expireTorrent;


};


#endif
