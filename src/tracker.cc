#include "tracker.h"
#include "request.h"
#include <boost/bind.hpp>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>


using namespace std;
using namespace v8;

MYSQL mysql;

Handle<Object> wrapRequest(request &req){
	Handle<Object> retf = Object::New();
	char *ip = inet_ntoa(*(struct in_addr *)&req.ip);
	retf->Set(String::New("left"),Number::New(req.left));
	retf->Set(String::New("corrupt"),Number::New(req.corrupt));
	retf->Set(String::New("downloaded"),Number::New(req.download));
	retf->Set(String::New("uploaded"),Number::New(req.upload));
	retf->Set(String::New("numwant"),Number::New(req.numwant));
	retf->Set(String::New("port"),Number::New(req.port));
	retf->Set(String::New("passkey"),String::New(req.pass));
	retf->Set(String::New("infohash"),String::New(req.torrent,20));
	retf->Set(String::New("peerid"),String::New(req.peerid,20));
	retf->Set(String::New("ip"),String::New(ip));
	if(req.event == req.UPDATE){
		retf->Set(String::New("event"),String::New("update"));
	}else if(req.event == req.START){
		retf->Set(String::New("event"),String::New("started"));
	}else if(req.event == req.STOP){
		retf->Set(String::New("event"),String::New("stopped"));
	}else if(req.event == req.COMPLETE){
		retf->Set(String::New("event"),String::New("completed"));
	}
	
	return retf;
}
Handle<Object> wrapTorrent(torrent *tor){
		Handle<Object> rett = Object::New();
		rett->Set(String::New("complete"),Number::New(tor->seeders));
		rett->Set(String::New("incomplete"),Number::New(tor->hosts - tor->seeders));
		rett->Set(String::New("download"),Number::New(tor->download));
		rett->Set(String::New("lastseen"),Date::New(tor->lastseen));
		rett->Set(String::New("infohash"),String::New(tor->id,20));
		return rett;
}

Handle<Object> wrapPeer(peer *pee){
	Handle<Object> rett = Object::New();
	char *ip = inet_ntoa(*(struct in_addr *)&pee->ip);
	rett->Set(String::New("left"),Number::New(pee->left));
	rett->Set(String::New("downloaded"),Number::New(pee->download));
	rett->Set(String::New("uploaded"),Number::New(pee->upload));
	rett->Set(String::New("corrupt"),Number::New(pee->corrupt));
	rett->Set(String::New("lastseen"),Date::New(pee->lastseen));
	rett->Set(String::New("passkey"),String::New(pee->pass));
	rett->Set(String::New("peerid"),String::New(pee->id,20));
	rett->Set(String::New("ip"),String::New(ip));
	rett->Set(String::New("port"),Number::New(pee->port));	
	return rett;
}

void tracker::removePeer(torrent *tor,peer *pee){
			tor->peers.erase(pee->id);
		if(pee->left == 0){
			if(tor->seeders > 0) tor->seeders--;
			if(seeders > 0) seeders--;
		}
		if(tor->hosts > 0) tor->hosts--;
		if(hosts > 0) hosts--;
		delete pee;

}
int tracker::peerList(request &req,torrent *tor,char *outb,size_t outs){
	//ok........
		int tt;
		int rstart=0;
		map<const char *,peer *>::iterator pees = tor->peers.begin();
		
		 if(req.numwant < tor->peers.size()){ 	
		 	rstart = rand() % (tor->peers.size() - req.numwant);
			for(int i=0;i<rstart;i++)
			pees++; 	
		}
				
		
tt = snprintf(outb,outs,"d8:completei%lde10:incompletei%lde8:intervali%de12:min intervali%de5:peers",
			tor->seeders,tor->hosts - tor->seeders, 300 , 120);
		
		if(req.compact){
			string walker;
			for(unsigned int i=0;i<req.numwant && pees != tor->peers.end();i++,pees++){
				
				peer *pi = pees->second;
				walker+= (char) ((pi->ip>>24)&255);
				walker+= (char) ((pi->ip>>16)&255);
				walker+= (char) ((pi->ip>>8)&255);
				walker+= (char)(pi->ip & 255);
				walker += (char)((pi->port & 0xff00) >> 8);
				walker += (char)((pi->port & 0xff));
			}
		char *p = outb + tt;
		int add;
		add=sprintf(p,"%d:",walker.size());
		p+=add;
		tt +=add;
		memcpy(p,walker.c_str(),walker.size());
		p+=walker.size();
		tt+=walker.size();
		*p='e';
		tt++;	
		}else{
			//not compact
			string walker;
			char *ip;
			walker +='l';
			for(unsigned int i=0;i<req.numwant && pees != tor->peers.end();i++,pees++){	
				peer *pi = pees->second;
				walker += "d2:ip";
				ip = inet_ntoa(*(struct in_addr *)&pi->ip);
				walker += boost::lexical_cast<std::string>(strlen(ip));
				walker += ':';
				walker += ip;
				walker += "4:porti";
				walker += boost::lexical_cast<std::string>(pi->port);
				walker += "ee";
			}
			walker +="ee";			
			char *p = outb + tt;
			memcpy(p,walker.c_str(),walker.size());
			tt += walker.size();
		}
		return tt;
		
}


void tracker::housekeeping(){ 
	//time in milisecs
	while(1){
		{
			boost::mutex::scoped_lock lock( io_mutex );	
				HandleScope handle_scope;
				Context::Scope context_scope(context);


			struct timeval tv;
			gettimeofday(&tv,NULL);
			double now = (static_cast<double>(tv.tv_sec) * 1000) + (static_cast<double>(tv.tv_usec) / 1000);
			map<const char *,torrent *>::iterator iterTor = torrents.begin();
			for(;iterTor != torrents.end();iterTor++){
				torrent *tor = iterTor->second;
				map<const char *,peer *>::iterator iterPeers = tor->peers.begin();
				for(;iterPeers != tor->peers.end();iterPeers++){
					peer *pee = iterPeers->second;
					if((pee->lastseen + 600000) < now){
						Handle<Object> jspee = wrapPeer(pee);
						Handle<Object> jstor = wrapTorrent(tor);
						const int argc = 2;
  						Handle<Value> argv[argc] = { jstor,jspee };	
						//run newRequest
						Handle<Value> result = expirePeer->Call(context->Global(), argc, argv);
						removePeer(tor,pee);
					} 
				}
				
			}
			
		}
		sleep(60);
	}
	
}

int tracker::scrape(request &req,char *outb,size_t outs){
	boost::mutex::scoped_lock lock( io_mutex );
	torrent *tor;
	map<const char *,torrent *>::iterator iter = torrents.find(req.torrent);
	if(iter == torrents.end()) return dumperr("not found",outb,outs);
	tor = iter->second;
	int add = 0 ;
	add = sprintf(outb,"d5:filesd20:");
	
	memcpy(outb + add,tor->id,20);
	add += 20;
	add += sprintf(outb + add,"d8:completei%lde10:downloadedi%lde10:incompletei%ldeeee",
		tor->seeders,tor->download,tor->hosts - tor->seeders);
	return add;        
}
	
//return number of bytes to send to client
//don't run strlen(outb) !
int tracker::processRequest(request &req,char *outb,size_t outs){
	boost::mutex::scoped_lock lock( io_mutex );
	HandleScope handle_scope;
	Context::Scope context_scope(context);
	struct timeval tv;
	gettimeofday(&tv,NULL);
	
	Handle<Object> retf = wrapRequest(req);
	
	map<const char *,torrent *>::iterator iter = torrents.find(req.torrent);
	torrent *tor = NULL;
	peer *pee = NULL;
	
	if(iter != torrents.end()){
		tor = iter->second;
		retf->Set(String::New("newTorrent"),Boolean::New(false));
		
		retf->Set(String::New("torrent"),wrapTorrent(tor));
		//maybe expose the torrent object later?
		map<const char *,peer *>::iterator iter1 = tor->peers.find(req.peerid);
		if(iter1 != tor->peers.end()){
			pee = iter1->second;
			retf->Set(String::New("newPeer"),Boolean::New(false));
			
			retf->Set(String::New("peer"),wrapPeer(pee));
		}else{
			retf->Set(String::New("newPeer"),Boolean::New(true));
		}
	}else{
		retf->Set(String::New("newTorrent"),Boolean::New(true));	
		retf->Set(String::New("newPeer"),Boolean::New(true));
	}

	
	const int argc = 1;
  	Handle<Value> argv[argc] = { retf };
	
	//run newRequest
	Handle<Value> result = newRequest->Call(context->Global(), argc, argv);		
	if(!result->IsUndefined()){
		v8::String::AsciiValue str(result);
		strncat(outb,*str,outs);
		return  dumperr(*str,outb,outs);
	}
	
	if(tor == NULL){
		tor = new torrent(req.torrent);
		torrents.insert(make_pair(tor->id,tor));
	
	}
	
	if(pee == NULL){
		pee = new peer(req.peerid);
		pee->ip = req.ip;
		pee->port = req.port;
		tor->peers.insert(make_pair(pee->id,pee));
		memcpy(pee->pass,req.pass,strlen(req.pass));
		tor->hosts++;
		if(req.left == 0){
			 tor->seeders++;
			 seeders++;
		}
		hosts++;
	}
	tor->lastseen = pee->lastseen = (static_cast<double>(tv.tv_sec) * 1000) + (static_cast<double>(tv.tv_usec) / 1000);
	pee->left = req.left;
	pee->corrupt = req.corrupt;
	pee->download = req.download;
	pee->upload = req.upload;
		
	if(req.event == req.COMPLETE){
		if(req.left == 0){
			tor->download++;
			download++;
			tor->seeders++;
			seeders++;
		}
	}else if(req.event == req.STOP){
		removePeer(tor,pee);
	}
	
	return peerList(req,tor,outb,outs);
		
}



const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}
Handle<Value> Print(const v8::Arguments& args){
	  bool first = true;
  for (int i = 0; i < args.Length(); i++) {
    v8::HandleScope handle_scope;
    if (first) {
      first = false;
    } else {
      printf(" ");
    }
    v8::String::Utf8Value str(args[i]);
    const char* cstr = ToCString(str);
    printf("%s", cstr);
  }
  printf("\n");
  fflush(stdout);
  return v8::Undefined();

}

Handle<String> ReadFile(const string& name) {
  HandleScope handle_scope;
  FILE* file = fopen(name.c_str(), "rb");
  if (file == NULL) return Handle<String>();

  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  rewind(file);

  char* chars = new char[size + 1];
  chars[size] = '\0';
  for (int i = 0; i < size;) {
    int read = fread(&chars[i], 1, size - i, file);
    i += read;
  }
  fclose(file);
  Handle<String> result = String::New(chars, size);
  delete[] chars;
  return handle_scope.Close(result);
}


//this is intended to return a single tuple like
// result.id = 123
// result.name = lucas
// result.download = 123
Handle<Value> mysqlQuery(const v8::Arguments& args){
	MYSQL_RES *res;
	MYSQL_ROW row;
    MYSQL_FIELD *fields;
    char *k;
    char *v;
	int nfields;
	Handle<Value> retb;
	
	if(args.Length() != 1){
		retb=v8::Undefined();
		return retb;
	}
	v8::String::Utf8Value str(args[0]);
    const char* cstr = ToCString(str);
	
 	mysql_query(&mysql,cstr);	

	res = mysql_use_result(&mysql);
    nfields = mysql_num_fields(res);
    fields = mysql_fetch_fields(res);

  if((row = mysql_fetch_row(res))){
  	Handle<Object> retf = Object::New();
                for(int i=0;i<nfields;i++){
                        k = fields[i].name;
                        v = row[i];
    					retf->Set(String::New(k),String::New(v));                    
                }	
	retb = retf;
  }else{
  	retb=v8::Undefined();
  }
	mysql_free_result(res);
	return retb;
}

Handle<ObjectTemplate> tracker::makeFuncs() {
  HandleScope handle_scope;

  Handle<ObjectTemplate> result = ObjectTemplate::New();
  result->Set(String::New("print"), FunctionTemplate::New(Print));
  result->Set(String::New("mysqlQuery"), FunctionTemplate::New(mysqlQuery));
 
  // Again, return the result through the current handle scope.
  return handle_scope.Close(result);
}


tracker::tracker(){
	HandleScope handle_scope;

	Handle<ObjectTemplate> funcs = makeFuncs();
	global =  Persistent<ObjectTemplate>::New(funcs);	
	
	Handle<Context> lcontext = Context::New(NULL, global);
	context = Persistent<Context>::New(lcontext);
	
	srand ( time(NULL) );
	
	Context::Scope context_scope(context);
	
    
 	Handle<String> source = ReadFile("lbtt.js");
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
  	
  	
  	//initiate mysql
  	mysql_init(&mysql);
  	mysql_real_connect(&mysql, "localhost","tb", "tr4ck3r", "bittorrent", MYSQL_PORT, NULL, 0);
	char a0 = true;
    mysql_options(&mysql, MYSQL_OPT_RECONNECT, &a0);
    
    seeders = hosts = download = 0;
    //housekeeping thread
    boost::thread thr(boost::bind(&tracker::housekeeping,this));
}

void tracker::status(ostream &fo){
	boost::mutex::scoped_lock lock( io_mutex );
	
	fo << "{ " ;
	fo << "\"torrents\": " << torrents.size() << "," ;
	fo << "\"peers\": " << hosts << "," ;
	fo << "\"seeders\": " << seeders << ",";
	fo << "\"downloads\": " << download << " }" ;
}

void tracker::info(ostream &fo,char *toc){
	boost::mutex::scoped_lock lock( io_mutex );
	torrent *tor;
	map<const char *,torrent *>::iterator iter = torrents.find(toc);
	if(iter == torrents.end()){ fo << "{\"error\": \"not found\"}" ; return ; }
	tor = iter->second;
	fo << "{ " ;
	fo << "\"complete\": " << tor->seeders << "," ;
	fo << "\"downloaded\": " << tor->download << "," ;
	fo << "\"incomplete\": " << (tor->hosts - tor->seeders) << " }";
}


int tracker::dumperr(const char *msg,char *outb,size_t outs){
	snprintf(outb,outs,"d14:failure reason%d:%se",strlen(msg),msg);
	return strlen(outb) ;
}
