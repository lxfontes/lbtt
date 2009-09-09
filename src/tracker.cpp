#include "tracker.h"
#include "scoped_lock.h"
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <v8.h>

using namespace std;
using namespace v8;

bool TorrentTracker::cleanup() {
    scoped_lock lock(io_mutex);
    HandleScope handle_scope;
    Context::Scope context_scope(context);

     struct timeval tv;
    gettimeofday(&tv, NULL);


    double now = (static_cast<double> (tv.tv_sec) * 1000) + (static_cast<double> (tv.tv_usec) / 1000);
	
    map<const char *, TorrentFile *>::iterator iterTor = torrents.begin();
    while (iterTor != torrents.end()) {
        TorrentFile *tor = iterTor->second;
        map<const char *, TorrentPeer *>::iterator iterPeers = tor->peers.begin();
        //check expired peers
        while (iterPeers != tor->peers.end()) {
            TorrentPeer *pee = iterPeers->second;
            if ((pee->lastseen + expireTimeout) < now) {
			Local<Object> torObj = torTemplate->NewInstance();
			torObj->SetInternalField(0, External::New(tor));
			Local<Object> peerObj = peerTemplate->NewInstance();
			peerObj->SetInternalField(0,External::New(pee));
			{
        const int argc = 2;
        Handle<Value> argv[argc] = {torObj,peerObj};
        //run newRequest
        Handle<Value> result = expirePeer->Call(context->Global(), argc, argv);
		}
			
                removePeer(tor, pee);
                iterPeers = tor->peers.begin();
            } else {
                iterPeers++;
            }
        }
        //check if torrent is expired
        if ((tor->lastseen + expireTimeout) < now && tor->peers.size() == 0) {
		Local<Object> torObj = torTemplate->NewInstance();
			torObj->SetInternalField(0, External::New(tor));
						{
        const int argc = 1;
        Handle<Value> argv[argc] = {torObj};
        //run newRequest
        Handle<Value> result = expirePeer->Call(context->Global(), argc, argv);
		}

            removeTorrent(tor);
            iterTor = torrents.begin();
        } else {
            iterTor++;
        }

    }
    return true;
}

TorrentTracker::TorrentTracker(string &path) :
hosts(0), downloads(0), seeders(0), interval(1800), expireTimeout(2000) {
    pthread_mutex_init(&io_mutex, NULL);
    startTime = time(NULL);
    scoped_lock lock(io_mutex);

    HandleScope handle_scope;


    Handle<ObjectTemplate> funcs = makeFuncs();

    global = Persistent<ObjectTemplate>::New(funcs);
    Handle<Context> lcontext = Context::New(NULL, global);
    context = Persistent<Context>::New(lcontext);
    Context::Scope context_scope(context);

    Handle<String> v8script = ReadFile(path.c_str());
    Handle<Script> script = Script::Compile(v8script, String::New(path.c_str()));

    if (script.IsEmpty()) {
        cout << "error initializing script, aborting" << endl;
        exit(99);
    }
    Handle<Value> result = script->Run();


    reqTemplate = Persistent<ObjectTemplate>::New(makeRequestTemplate());
    torTemplate = Persistent<ObjectTemplate>::New(makeTorrentTemplate());
    peerTemplate = Persistent<ObjectTemplate>::New(makePeerTemplate());

    Handle<String> process_name = String::New("newRequest");
    Handle<Value> process_val = context->Global()->Get(process_name);

    if (process_val->IsFunction()) {
        Handle<Function> hfun = Handle<Function>::Cast(process_val);
        newRequest = Persistent<Function>::New(hfun);
    }
    process_name = String::New("expirePeer");
    process_val = context->Global()->Get(process_name);

    if (process_val->IsFunction()) {
        Handle<Function> hfun = Handle<Function>::Cast(process_val);
        expirePeer = Persistent<Function>::New(hfun);
    }
    process_name = String::New("expireTorrent");
    process_val = context->Global()->Get(process_name);

    if (process_val->IsFunction()) {
        Handle<Function> hfun = Handle<Function>::Cast(process_val);
        expireTorrent = Persistent<Function>::New(hfun);
    }

}

void TorrentTracker::removeTorrent(TorrentFile *tor) {
    torrents.erase(tor->id);
    delete tor;
}

void TorrentTracker::removePeer(TorrentFile *tor, TorrentPeer *pee) {
    tor->peers.erase(pee->id);

    vector<TorrentPeer *>::iterator vit = tor->peerCache.begin();
    for (; vit != tor->peerCache.end(); vit++) {
        TorrentPeer *p = *vit;
        if (memcmp(p->id, pee->id, 20) == 0) {
            tor->peerCache.erase(vit);
            break;
        }
    }

    if (pee->state == TorrentPeer::SEED) {
        if (tor->seeders > 0) tor->seeders--;
        if (seeders > 0) seeders--;
    }
    if (tor->hosts > 0) tor->hosts--;
    if (hosts > 0) hosts--;
    delete pee;
}

void TorrentTracker::setError(stringstream &output, const char *x) {
    output << "d14:failure reason" << static_cast<int> (strlen(x)) << ":" << x << "e";
}

void TorrentTracker::peerList(TorrentRequest &req, TorrentFile *tor, stringstream &output) {

    vector<TorrentPeer *>::iterator pees;
    if (req.numwant > tor->peerCache.size()) {
        pees = tor->peerCache.begin();
    } else {
        int rstart = 0;
        pees = tor->peerCache.begin();
        rstart = (int) rand() % (tor->peerCache.size() - (req.numwant / 2));
        pees += rstart;
    }

    output << "d8:completei" << tor->seeders << "e10:incompletei" << (tor->hosts - tor->seeders) <<
            "e8:intervali" << interval << "e12:min intervali" << (interval / 2) << "e5:peers";

    if (req.compact) {
        string walker;
        for (unsigned int i = 0; i < req.numwant && pees != tor->peerCache.end(); i++, pees++) {
            TorrentPeer *pi = *pees;
            walker += (char) ((pi->ip >> 24)&255);
            walker += (char) ((pi->ip >> 16)&255);
            walker += (char) ((pi->ip >> 8)&255);
            walker += (char) (pi->ip & 255);
            walker += (char) ((pi->port & 0xff00) >> 8);
            walker += (char) ((pi->port & 0xff));
        }
        output << walker.size() << ":" << walker << "e";
    } else {
        //not compact
        stringstream walker;
        char *ip;
        walker << 'l';
        for (unsigned int i = 0; i < req.numwant && pees != tor->peerCache.end(); i++, pees++) {
            TorrentPeer *pi = *pees;
            walker << "d2:ip";
            ip = inet_ntoa(*(struct in_addr *) & pi->ip);
            walker << strlen(ip);
            walker << ':';
            walker << ip;
            walker << "4:porti";
            walker << pi->port;
            walker << "ee";
        }
        walker << "ee";
        output << walker;
    }
}

bool TorrentTracker::announce(TorrentRequest& req, stringstream& output) {
    scoped_lock lock(io_mutex);
    HandleScope handle_scope;
    Context::Scope context_scope(context);
    bool newPeer = true;
	bool newTorrent = true;
    TorrentFile *tor = NULL;
    TorrentPeer *peer = NULL;
    map<const char *, TorrentFile *>::iterator iter = torrents.find(req.torrent);


    Local<Object> reqObj = reqTemplate->NewInstance();
    reqObj->SetInternalField(0, External::New(&req));


    if (iter != torrents.end()) {
        tor = iter->second;
		newTorrent = false;
        reqObj->Set(String::New("newTorrent"), Boolean::New(false));
        Local<Object> torObj = torTemplate->NewInstance();
        torObj->SetInternalField(0, External::New(tor));
        reqObj->Set(String::New("torrent"), torObj);

        map<const char *, TorrentPeer *>::iterator iterPeer = tor->peers.find(req.peerid);
        if (iterPeer != tor->peers.end()) {
            //found peer
			newPeer = false;
            peer = iterPeer->second;
            reqObj->Set(String::New("newPeer"), Boolean::New(false));
            Local<Object> peerObj = peerTemplate->NewInstance();
            peerObj->SetInternalField(0, External::New(peer));
            reqObj->Set(String::New("peer"), peerObj);
        } else {
            reqObj->Set(String::New("newPeer"), Boolean::New(true));
        }

    } else {
        reqObj->Set(String::New("newTorrent"), Boolean::New(true));
        reqObj->Set(String::New("newPeer"), Boolean::New(true));
    }

    // run function
    {
        const int argc = 1;
        Handle<Value> argv[argc] = {reqObj};
        //run newRequest
        Handle<Value> result = newRequest->Call(context->Global(), argc, argv);
        if (!result->IsUndefined()) {
            v8::String::AsciiValue str(result);
            setError(output, *str);
            return false;
        }

    }


    if (tor == NULL) {
        tor = new TorrentFile(req.torrent);
        torrents.insert(make_pair(tor->id, tor));
    }


    if (peer == NULL) {
        peer = new TorrentPeer(req.peerid);
        peer->ip = req.ip;
        peer->port = req.port;
        tor->peers.insert(make_pair(peer->id, peer));
        tor->peerCache.push_back(peer);
        tor->hosts++;
        hosts++;
        if (req.left == 0) {
            tor->seeders++;
            seeders++;
			peer->state = TorrentPeer::SEED;
        }
    }
    peer->left = req.left;
    peer->corrupt = req.corrupt;
    peer->download = req.download;
    peer->upload = req.upload;

    struct timeval tv;
    gettimeofday(&tv, NULL);


    tor->lastseen = peer->lastseen = (static_cast<double> (tv.tv_sec) * 1000) + (static_cast<double> (tv.tv_usec) / 1000);


    if (req.event == req.COMPLETE) {
        if (peer->left == 0 && newPeer == false && peer->state == TorrentPeer::LEECH) { //add peer state to avoid people sending complete = complete
		    tor->seeders++;
			seeders++;
            tor->downloads++;
            downloads++;
        }
    } else if (req.event == req.STOP) {
        removePeer(tor, peer);
    }else{
	
	}

    peerList(req, tor, output);

    return true;
}

bool TorrentTracker::scrape(TorrentRequest& req, stringstream& output) {
    scoped_lock lock(io_mutex);
    TorrentFile *tor;

    map<const char *, TorrentFile *>::iterator iter = torrents.find(req.torrent);

    if (iter == torrents.end()) {
        setError(output, "torrent not found");
        return false;
    }
    tor = iter->second;

    output << "d5:filesd20:";
    output.write(req.torrent, 20);
    output << "d8:completei" << tor->seeders << "e10:downloadedi" << tor->downloads <<
            "e10:incompletei" << (tor->hosts - tor->seeders) << "eeee";

    return true;
}

bool TorrentTracker::info(TorrentRequest& req, stringstream& output) {
		scoped_lock lock( io_mutex );
        TorrentFile *tor;
        map<const char *,TorrentFile *>::iterator iter = torrents.find(req.torrent);
        if(iter == torrents.end()){ output << "{\"error\": \"not found\"}" ; return false; }
        tor = iter->second;
        output << "{ " ;
        output << "\"complete\": " << tor->seeders << "," ;
        output << "\"downloaded\": " << tor->downloads << "," ;
        output << "\"incomplete\": " << (tor->hosts - tor->seeders) << " }";

    return true;
}

void TorrentTracker::status(stringstream& output) {
    scoped_lock lock(io_mutex);
    time_t now = time(NULL);
    time_t delta = now - startTime;
    output << "{ ";
    output << "\"uptime\": " << delta << ",";
    output << "\"torrents\": " << torrents.size() << ",";
    output << "\"peers\": " << hosts << ",";
    output << "\"seeders\": " << seeders << ",";
    output << "\"downloads\": " << downloads << " }";
}

