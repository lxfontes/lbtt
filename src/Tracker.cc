
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "thread.hpp"
#include "scoped_lock.hpp"
#include "Tracker.h"
#include "v8funcs.h"
using namespace std;
using namespace v8;


void Tracker::removeTorrent(Torrent *tor){
	torrents.erase(tor->id);
	delete tor;
}

void Tracker::removePeer(Torrent *tor,Peer *pee){
	tor->peers.erase(pee->id);
	if(pee->left == 0){
		if(tor->seeders > 0) tor->seeders--;
		if(seeders > 0) seeders--;
	}
	if(tor->hosts > 0) tor->hosts--;
	if(hosts > 0) hosts--;
	delete pee;
}
void Tracker::setError(stringstream &output,const char *x){
	output << "d14:failure reason" << static_cast<int>(strlen(x)) << ":" << x << "e";
}
void Tracker::scrape(Request &req,stringstream &output){
	scoped_lock lock( io_mutex );
	Torrent *tor;
	
	map<const char *,Torrent *>::iterator iter = torrents.find(req.torrent);
	
	if(iter == torrents.end()){
		setError(output,"not found"); return;
	}
	tor = iter->second;
	
	
	output << "d5:filesd20:";
	output.write(req.torrent,20) ;
	output << "d8:completei" << tor->seeders << "e10:downloadedi" << tor->download <<
	"e10:incompletei" << (tor->hosts - tor->seeders) << "eeee";
}


void Tracker::peerList(Request &req,Torrent *tor,stringstream &output){
	
	int rstart=0;
	map<const char *,Peer *>::iterator pees = tor->peers.begin();

	if(req.numwant < tor->peers.size()){
		rstart = rand() % (tor->peers.size() - req.numwant);
		for(int i=0;i<rstart;i++)
		pees++;
	}

	output << "d8:completei" << tor->seeders << "e10:incompletei" << (tor->hosts - tor->seeders) <<
	"e8:intervali" << interval << "e12:min intervali" << (interval/2) << "e5:peers";

	if(req.compact){
		string walker;
		for(unsigned int i=0;i<req.numwant && pees != tor->peers.end();i++,pees++){

			Peer *pi = pees->second;
			walker+= (char) ((pi->ip>>24)&255);
			walker+= (char) ((pi->ip>>16)&255);
			walker+= (char) ((pi->ip>>8)&255);
			walker+= (char)(pi->ip & 255);
			walker += (char)((pi->port & 0xff00) >> 8);
			walker += (char)((pi->port & 0xff));
		}
		output << walker.size() << ":" << walker << "e";
	}else{
		//not compact
		stringstream walker;
		char *ip;
		walker << 'l';
		for(unsigned int i=0;i<req.numwant && pees != tor->peers.end();i++,pees++){
			Peer *pi = pees->second;
			walker << "d2:ip";
			ip = inet_ntoa(*(struct in_addr *)&pi->ip);
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

void Tracker::announce(Request &req,stringstream &output){
	scoped_lock lock( io_mutex );
	HandleScope handle_scope;
	Context::Scope context_scope(context);

	struct timeval tv;
	gettimeofday(&tv,NULL);


	Handle<Object> requestJs = wrapRequest(req);

	Torrent *tor = NULL;
	Peer *peer = NULL;
	map<const char *,Torrent *>::iterator iter = torrents.find(req.torrent);

	if(iter != torrents.end()){
		tor = iter->second;
		requestJs->Set(String::New("newTorrent"),Boolean::New(false));
		requestJs->Set(String::New("torrent"),wrapTorrent(tor));

		map<const char *,Peer *>::iterator iterPeer = tor->peers.find(req.peerid);
		if(iterPeer != tor->peers.end()){
			//found peer
			peer = iterPeer->second;
			requestJs->Set(String::New("newPeer"),Boolean::New(false));
			requestJs->Set(String::New("peer"),wrapPeer(peer));
		}else{
			//set peer for creation
			requestJs->Set(String::New("newPeer"),Boolean::New(true));
		}
		
	}else{
		//torrent not found, set it for creation
		requestJs->Set(String::New("newPeer"),Boolean::New(true));
		requestJs->Set(String::New("newTorrent"),Boolean::New(true));
		
	}
	
	const int argc = 1;
	Handle<Value> argv[argc] = { requestJs };

	//run newRequest
	Handle<Value> result = newRequest->Call(context->Global(), argc, argv);
	if(!result->IsUndefined()){
		v8::String::AsciiValue str(result);
		setError(output,*str);
		return;
	}

	if(tor == NULL){
		tor = new Torrent(req.torrent);
		torrents.insert(make_pair(tor->id,tor));
	}
	
	if(peer == NULL){
		peer = new Peer(req.peerid);
		peer->ip = req.ip;
		peer->port = req.port;
		peer->pass = req.pass;
		tor->peers.insert(make_pair(peer->id,peer));
		tor->hosts++;
		hosts++;
		if(req.left == 0){
			tor->seeders++;
			seeders++;
		}
	}
	
	peer->left = req.left;
	peer->corrupt = req.corrupt;
	peer->download = req.download;
	peer->upload = req.upload;


	tor->lastseen = peer->lastseen = (static_cast<double>(tv.tv_sec) * 1000) + (static_cast<double>(tv.tv_usec) / 1000);

	if(req.event == req.COMPLETE){
		if(peer->left == 0){ //add peer state to avoid people sending complete = complete
			tor->download++;
			download++;
			tor->seeders++;
			seeders++;
		}
	}else if(req.event == req.STOP){
		removePeer(tor,peer);
	}

	peerList(req,tor,output);

}


void Tracker::run(){
	while(1){
		{
			scoped_lock lock(io_mutex);
			HandleScope handle_scope;
			Context::Scope context_scope(context);


			struct timeval tv;
			gettimeofday(&tv,NULL);
			double now = (static_cast<double>(tv.tv_sec) * 1000) + (static_cast<double>(tv.tv_usec) / 1000);
			map<const char *,Torrent *>::iterator iterTor = torrents.begin();
			for(;iterTor != torrents.end();iterTor++){
				
				Torrent *tor = iterTor->second;
				map<const char *,Peer *>::iterator iterPeers = tor->peers.begin();
				//check expired peers
				for(;iterPeers != tor->peers.end();iterPeers++){
					Peer *pee = iterPeers->second;
					if((pee->lastseen + expireTimeout) < now){
						Handle<Object> jspee = wrapPeer(pee);
						Handle<Object> jstor = wrapTorrent(tor);
						const int argc = 2;
						Handle<Value> argv[argc] = { jstor,jspee };
						Handle<Value> result = expirePeer->Call(context->Global(), argc, argv);
						
						removePeer(tor,pee);
					}
				}
				//check if torrent is expired
				if((tor->lastseen + expireTimeout) < now && tor->peers.size() == 0 ){
					Handle<Object> jstor = wrapTorrent(tor);
					const int argc = 1;
					Handle<Value> argv[argc] = { jstor };
					Handle<Value> result = expireTorrent->Call(context->Global(), argc, argv);
					
					removeTorrent(tor);
				}

			}

		}
		sleep(cleanupInterval);
	}

}



Tracker::Tracker(string &scriptFile): interval(300),seeders(0),hosts(0),download(0){
	pthread_mutex_init(&io_mutex,NULL);
	srand ( time(NULL) );
	HandleScope handle_scope;

	Handle<ObjectTemplate> funcs = makeFuncs();
	global =  Persistent<ObjectTemplate>::New(funcs);

	Handle<Context> lcontext = Context::New(NULL, global);
	context = Persistent<Context>::New(lcontext);

	Context::Scope context_scope(context);

	Handle<String> source = ReadFile(scriptFile);
	Handle<Script> script = Script::Compile(source);
	Handle<Value> result = script->Run();

	Handle<String> process_name = String::New("newRequest");
	Handle<Value> process_val = context->Global()->Get(process_name);

	if (process_val->IsFunction()){
		Handle<Function> hfun = Handle<Function>::Cast(process_val);
		newRequest = Persistent<Function>::New(hfun);
	}

	process_name = String::New("expirePeer");
	process_val = context->Global()->Get(process_name);

	if (process_val->IsFunction()){
		Handle<Function> hfun = Handle<Function>::Cast(process_val);
		expirePeer = Persistent<Function>::New(hfun);
	}
	process_name = String::New("expireTorrent");
	process_val = context->Global()->Get(process_name);

	if (process_val->IsFunction()){
		Handle<Function> hfun = Handle<Function>::Cast(process_val);
		expireTorrent = Persistent<Function>::New(hfun);
	}


}


void Tracker::status(ostream &fo){
	scoped_lock lock( io_mutex );

	fo << "{ " ;
	fo << "\"torrents\": " << torrents.size() << "," ;
	fo << "\"peers\": " << hosts << "," ;
	fo << "\"seeders\": " << seeders << ",";
	fo << "\"downloads\": " << download << " }" ;
}

void Tracker::info(ostream &fo,char *toc){
	scoped_lock lock( io_mutex );
	Torrent *tor;
	map<const char *,Torrent *>::iterator iter = torrents.find(toc);
	if(iter == torrents.end()){ fo << "{\"error\": \"not found\"}" ; return ; }
	tor = iter->second;
	fo << "{ " ;
	fo << "\"complete\": " << tor->seeders << "," ;
	fo << "\"downloaded\": " << tor->download << "," ;
	fo << "\"incomplete\": " << (tor->hosts - tor->seeders) << " }";
}


