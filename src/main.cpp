#include "tracker.h"
#include "acceptor.h"
#include "thread.h"
#include "worker.h"
#include <v8.h>
#include <iostream>
#include <ev.c>
#include <signal.h>
#include <vector>
#include <getopt.h>
using namespace std;
using namespace v8;

vector<Worker *> workers;
vector<thread<Worker> *> tworkers;
vector<Worker *>::iterator workerit;
void newClient(int cfd) {
    Worker *w = *workerit;
    w->newClient(cfd);
    workerit++;
    if(workerit == workers.end())
        workerit = workers.begin();
}
bool running;

void stopme(int s){
    running = false;
}


int printHelp(){
        cout << "-h\thelp" << endl;
        cout << "-t\tthreads [def. 5]" << endl;
        cout << "-p\tport [def. 8080]" << endl;
        cout << "-b\tip [def. 0.0.0.0]" << endl;
        cout << "-s\tjavascript file [def. lbtt.js]" << endl;
        cout << "-i\tannounce interval [def. 900]" << endl;
        cout << "-e\texpire interval [def. 1800]" << endl;
        return 0;
}


int main(int argc, char **argv) {

    int nthreads = 5;
    int socket_timeout = 5;
    int port = 8080;
	int interval = 900;
	int expire = (interval * 2);
	string script("lbtt.js");
	string bindip("0.0.0.0");
	char opt;


	     

        while((opt = getopt(argc, argv, "i:e:ht:p:b:s:")) != -1) {
                switch(opt){
                case 'i':
                        interval = atoi(optarg);
                        break;
                case 'e':
                        expire = atoi(optarg);
                        break;
                case 'h':
                        return printHelp();
                case 't':
                        nthreads = atoi(optarg);
                        break;
                case 'p':
                        port = atoi(optarg);
                        break;
                case 'b':
                        bindip = optarg;
                        break;
                case 's':
                        script = optarg;
                        break;
                }
        }
      v8::V8::SetFlagsFromCommandLine(&argc, argv, true);

    TorrentTracker tracker(script);
    tracker.interval = interval;
    tracker.expireTimeout = expire * 1000;

    //bbye stdin
    close(0);

    Acceptor binder(bindip.c_str(), port);
    binder.setCallback(newClient);

    thread<Acceptor> binderthread(binder);
Worker *w;
    for (int i = 0; i < nthreads; i++) {
        w = new Worker(tracker);
        w->timeout = socket_timeout;
        thread<Worker> *twork = new thread<Worker>(*w);
        workers.push_back(w);
    }

    workerit = workers.begin();

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, stopme);
    running = true;
    cout << "started" << endl;
	int sleepcount = 0;
    while (running) {
	    if(sleepcount >= 30){
        tracker.cleanup();
		sleepcount = 0;
		}
        sleep(1);
		sleepcount++;
    }

    binder.stop();

    return 0;
}
