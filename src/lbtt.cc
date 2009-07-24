#include <getopt.h>
#include <ev.c>
#include <iostream>
#include <fcntl.h>
#include <ev++.h>

#include "acceptor.h"
#include "processor.h"
#include "tracker.h"
#include "sfcgi.h"
	using namespace std;
	
int printHelp(){
	cout << "-h\thelp" << endl;
	cout << "-t\tthreads [def. 10, max 50]" << endl;
	cout << "-p\tport [def. 8080]" << endl;
	cout << "-b\tip [def. 0.0.0.0]" << endl;
	cout << "-f\tfastcgi ip:port [def. 0.0.0.0:8082]" << endl;
	cout << "-s\tjavascript file [def. lbtt.js]" << endl;
		return 0;
}
int main(int argc,char **argv){
	char opt;
	const char *ip = "0.0.0.0";
	int port=8080;
	const char *fcgiport = ":8082";
	int threads = 10;
	const char *scriptFile = "lbtt.js";
	
	 while((opt = getopt(argc, argv, "ht:p:b:f:s:")) != -1) {
		switch(opt){
			
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
				scriptFile = optarg;
				break;
		}
	}
	if(threads > 50){
		cout << "limiting threads to 50" << endl;
		threads = 50;
	}
	cout << "starting @ " << ip << ":" << port << " fcgi " << fcgiport << " threads " << threads << endl;
	tracker lbtt(scriptFile);
	sfcgi cgi(lbtt,fcgiport);
	processor http(lbtt);
	acceptor ac(ip,port,threads,http);
	ac.run();
	return 0;
}
