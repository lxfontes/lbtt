#include "ev.c"
#include "Acceptor.h"
#include "Tracker.h"
#include "FastCgi.h"
#include "thread.hpp"
#include <iostream>
#include <getopt.h>

using namespace std;

int printHelp(){
	cout << "-h\thelp" << endl;
	cout << "-t\tthreads [def. 10, max 50]" << endl;
	cout << "-p\tport [def. 8080]" << endl;
	cout << "-b\tip [def. 0.0.0.0]" << endl;
	cout << "-f\tfastcgi ip:port [def. 127.0.0.1:8082]" << endl;
	cout << "-s\tjavascript file [def. lbtt.js]" << endl;
	cout << "-i\tannounce interval [def. 300]" << endl;
	cout << "-e\texpire interval [def. 600]" << endl;
	return 0;
}

int main(int argc,char **argv){
	string ip = "0.0.0.0";
	string script = "lbtt.js";
	char *fcgiport = "127.0.0.1:8082";
	int threads = 10;
	int port = 8080;
	int interval = 300;
	int expire = 600000;
	char opt;

	while((opt = getopt(argc, argv, "i:e:ht:p:b:f:s:")) != -1) {
		switch(opt){
		case 'i':
			interval = atoi(optarg);
			break;
		case 'e':
			expire = atoi(optarg) * 1000;
			break;
		case 'h':
			return printHelp();
		case 'f':
			fcgiport = optarg;
		case 't':
			threads = atoi(optarg);
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 'b':
			ip = optarg;
			break;
		case 's':
			script = optarg;
			break;
		}
	}
	if(threads > 50){
		cout << "limiting threads to 50" << endl;
		threads = 50;
	}
	cout << "starting @ " << ip << ":" << port << " fcgi " << fcgiport << " threads " << threads << endl;

	
	signal(SIGPIPE, SIG_IGN);
	Tracker t(script);

	t.interval = interval;
	t.expireTimeout = expire;
	t.cleanupInterval = 60;
	Acceptor ac(t,ip,port,threads);
	FastCgi fcgi(t,fcgiport);
	thread<FastCgi> thrCgi(&fcgi);
	thread<Tracker> thrTracker(&t);
	//main loop
	ac.run();

	return 0;
}
