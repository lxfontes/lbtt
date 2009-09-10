/* 
 * File:   tracker.h
 * Author: lfontes
 *
 * Created on September 4, 2009, 4:31 PM
 */

#ifndef _TRACKER_H
#define	_TRACKER_H
#include "trackertypes.h"
#include <map>
#include <pthread.h>
#include <iostream>
#include "v8funcs.h"
#include <v8.h>

using namespace std;
using namespace v8;

class TorrentTracker {
public:
    TorrentTracker(string &);
    bool scrape(TorrentRequest &req, stringstream &output);
    bool announce(TorrentRequest &req, stringstream &output);
    void status(stringstream &output);
    bool info(TorrentRequest &req, stringstream &output);
	bool dynamic(TorrentRequest &req, stringstream &output);
    bool cleanup();
    int interval;
    int expireTimeout;
private:
    void removePeer(TorrentFile *, TorrentPeer *);
    void removeTorrent(TorrentFile *);

    void peerList(TorrentRequest &req, TorrentFile *torrent, stringstream &output,bool stop);
    void setError(stringstream &output, const char *msg);
protected:
    map<const char *, TorrentFile *, hashCmp> torrents;
    pthread_mutex_t io_mutex;
    unsigned long hosts;
    unsigned long seeders;
    unsigned long downloads;
    time_t startTime;

    //v8 specific
    Handle<Context> context;
    Persistent<ObjectTemplate> global;

    Persistent<Function> newRequest;
	Persistent<Function> dynamicRequest;
    Persistent<Function> expirePeer;
    Persistent<Function> expireTorrent;


    Persistent<ObjectTemplate> reqTemplate;
    Persistent<ObjectTemplate> torTemplate;
    Persistent<ObjectTemplate> peerTemplate;
};

#endif	/* _TRACKER_H */

