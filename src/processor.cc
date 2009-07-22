#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "sfcgi.h"
#include "processor.h"
#include "request.h"
	using namespace std;

int processor::dumperr(const char *err,char *outb,size_t outs){
	sprintf(outb,"HTTP/1.0 400 Bad Request\r\nContent-Type: text/html\r\n\r\nerror: %s\r\n",err);
	return strlen(outb);
}





int processor::process(int sockfd,char *inb,size_t ins,char *outb,size_t outs){
		bool scrape = false;
		 
		//easier to play with plain c char
		if(strncmp("GET",inb,3) != 0){
			cout << inb << endl;
			return dumperr("not GET",outb,outs);
		}
		
		char *query = NULL;
		char *proto = NULL;
		//process first line
		query = strchr(inb,' ');
		if(query == NULL || ((proto=strchr(query + 1,' ')) == NULL)){
			
			return dumperr("not get",outb,outs);
		}
		*proto = '\0';
		query++;
		
		//check path
		if(strncmp("/scrape",query,7) == 0){
			scrape = true;
		}else if(strncmp("/announce",query,9) != 0){
			return dumperr("not announce/scrape",outb,outs);
		}
		request req;
		req.init();
		if(strchr(query,'?') == NULL){
			return dumperr("no query string",outb,outs);
		}
		char *p,*argument = query;
		char bvalue[20];
		int nvalue;
		strsep(&argument,"?");
		p = argument;		
		
		if(p == NULL || *p == '\0')return dumperr("no query string",outb,outs);
		
		struct sockaddr_in mysock;
		socklen_t namelen;
		namelen = sizeof(struct sockaddr_in);
		getpeername(sockfd, (struct sockaddr *)&mysock, &namelen);
		req.ip = mysock.sin_addr.s_addr;

		
		while (p != NULL && *p != '\0') {
			char *key, *value;
			argument = strsep(&p, "&");

			value = argument;
			key = strsep(&value, "=");
			if (value == NULL)return dumperr("error parsing qstring",outb,outs);
			nvalue = sfcgi::decode_q(value,bvalue);
		
			if(strcmp("info_hash",key) == 0){
				//check length
				if(nvalue != 20)return dumperr("invalid hash size",outb,outs);
				memcpy(req.torrent,bvalue,20);
				req.valid++;
			}else if(strcmp("peer_id",key) == 0){
				if(nvalue != 20)return dumperr("invalid peer size",outb,outs);
				memcpy(req.peerid,bvalue,20);
				req.valid++;
			}else if(strcmp("left",key) == 0){
				req.left = atol(bvalue);
				req.valid++;
			}else if(strcmp("passkey",key) == 0){
				strncpy(req.pass,value,20); //don't use hex encoded in passkeys !
			}else if(strcmp("corrupt",key) == 0){
				req.corrupt = atol(bvalue);
				req.valid++;
			}else if(strcmp("downloaded",key) == 0){
				req.download = atol(bvalue);
				req.valid++;
			}else if(strcmp("uploaded",key) == 0){
				req.upload = atol(bvalue);
				req.valid++;
			}else if(strcmp("event",key) == 0){
				if(strcmp(bvalue,"started")==0){
					req.event = req.START;
				}else if(strcmp(bvalue,"stopped")==0){
					req.event = req.STOP;
				}else if(strcmp(bvalue,"completed")==0){
					req.event = req.COMPLETE;
				}
				req.valid++;
			}else if(strcmp("port",key)==0){
				req.port = atoi(bvalue);
				req.valid++;
			}else if(strcmp("numwant",key) ==0){
				req.numwant = atoi(bvalue);
				if(req.numwant > 30 || req.numwant < 1)
					req.numwant = 30;
				req.valid++;
			}else if(strcmp("compact",key) == 0){
				req.compact = true;
			}else if(strcmp("ip",key)==0){
				req.ip = inet_addr(bvalue);
				if(req.ip == INADDR_NONE)return dumperr("invalid peer size",outb,outs);
			}			
		}

		if(req.valid < 7 && scrape == false){
//			req.print();
			return dumperr("invalid request",outb,outs);
		}
		//put the http header in there
		snprintf(outb,outs,"HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n");
		if(scrape)
			return 44 + m_tracker.scrape(req,outb + 44,outs - 44);
		else
			return 44 + m_tracker.processRequest(req,outb + 44,outs - 44);

}
