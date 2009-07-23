#ifndef __TRACKER_H__
#define __TRACKER_H__
#include <map>
#include <mysql.h>
#include "request.h"
#include "scoped_lock.hpp"
#include <v8.h>
#include <iostream>
#include <ostream>
using namespace std;

using namespace v8;

struct hashCmp {
      bool operator()( const char *s1, const char *s2 ) const {
        return memcmp( s1, s2,20 ) < 0;
      }
    };
    
    
class peer{
public:
	peer(char *s){ memcpy(id,s,20); pass[0]='\0';left = ip = port = corrupt = download = upload = 0;}
	char id[20];
	unsigned long left;
	unsigned long corrupt;
	unsigned long download;
	unsigned long upload;
	int port;
	in_addr_t ip;
	char pass[20];
	double lastseen;
};

class torrent{
public:
	torrent(char *s):peers() {memcpy(id,s,20); hosts = seeders = download = 0;}
	char id[20];
	map<const char *, peer *, hashCmp > peers;
	unsigned long hosts;
	unsigned long seeders;
	unsigned long download;
	double lastseen;
};


class tracker{
public:
	tracker();
	int processRequest(request &,char *,size_t);
	int scrape(request &,char *,size_t);
	int dumperr(const char *,char *,size_t);
	void run();
	void status(ostream &);
	void info(ostream &,char *);

protected:
	Handle<ObjectTemplate> makeFuncs();
	pthread_mutex_t io_mutex ;

	//v8 specific
	Persistent<Context> context;
	Persistent<ObjectTemplate> global;
	
	Persistent<Function> newRequest;
	Persistent<Function> expirePeer;
	Persistent<Function> expireTorrent;
	
	//tracker
		void removePeer(torrent *,peer *);
	map<const char *,torrent *, hashCmp > torrents;
	int peerList(request &,torrent *,char *,size_t);
	
	unsigned long hosts;
	unsigned long seeders;
	unsigned long download;
	
};


#endif
