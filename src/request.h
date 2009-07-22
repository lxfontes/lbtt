#ifndef __REQUEST_H__
#define __REQUEST_H__
#include <netinet/in.h>

struct request{
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
	int port;
	in_addr_t ip;
	char pass[20];
	unsigned int numwant;
	int valid;
	void init(){ pass[0]='\0';valid  = port = ip  =0;event = UPDATE;compact=false;}
};

#endif
