#include "FastCgi.h"
#include "fcgio.h"
#include "fcgi_config.h"  // HAVE_IOSTREAM_WITHASSIGN_STREAMBUF
#include <iostream>
#include <fstream>

using namespace std;

FastCgi::FastCgi(Tracker &t,const char *port): m_tracker(t) {
	FCGX_Init();
	usock = FCGX_OpenSocket(port,10);
}

void FastCgi::run(){
	FCGX_Request request;
	FCGX_InitRequest(&request, usock, 0);
	while (FCGX_Accept_r(&request) == 0){
		fcgi_streambuf cin_fcgi_streambuf(request.in);
		fcgi_streambuf cout_fcgi_streambuf(request.out);
		fcgi_streambuf cerr_fcgi_streambuf(request.err);
		ostream fo(&cout_fcgi_streambuf);
		fo << "Content-Type: text/html\r\n\r\n";
		string path(FCGX_GetParam("REQUEST_URI",request.envp));
		char *query = FCGX_GetParam("QUERY_STRING",request.envp);
		int nquery = query!=NULL?strlen(query):0;
		size_t pos1;

		//status
		pos1 = path.find("/lbtt/status");
		if(pos1 != string::npos){
			if(nquery > 0){
				fo << query << "(";
			}
			m_tracker.status(fo);
			if(nquery > 0){ fo << ")";}
			continue;
		}
		//specific torrent status
		pos1 = path.find("/lbtt/info");
		if(pos1 != string::npos){
			string cb;
			char infohash[20];
			bool hashax = false;
			if(nquery <= 0) { fo << "invalid query";continue; }
			char *p;
			char *argument;
			char bvalue[20];
			int nvalue;
			p = query;
			while (p != NULL && *p != '\0') {
				char *key, *value;
				argument = strsep(&p, "&");
				value = argument;
				key = strsep(&value, "=");
				if (value == NULL){ fo << "invalid query";continue; }
				nvalue = decode_q(value,bvalue);
				if(strcmp(key,"info_hash") ==0){
					if(nvalue == 20){
						hashax = true;
						memcpy(infohash,bvalue,20);
					}
				}else if(strcmp(key,"cb")==0){
					cb += bvalue;
				}
			}
			if(hashax){
				if(cb.size() > 0 ) { fo << cb << "("; }
				m_tracker.info(fo,infohash);
				if(cb.size() > 0 ) { fo << ")"; }
			}else{
				fo << "invalid request";
			}
			continue;
		}

	}
}
