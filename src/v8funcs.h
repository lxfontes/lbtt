#ifndef __V8FUNCS__
#define __V8FUNCS__
#include <v8.h>
#include <iostream>
#include "Tracker.h"
using namespace v8;
using namespace std;

Handle<Object> wrapRequest(Request &);
Handle<Object> wrapTorrent(Torrent *);
Handle<Object> wrapPeer(Peer *);
Handle<Value> Print(const v8::Arguments&);
Handle<ObjectTemplate> makeFuncs();
Handle<String> ReadFile(const string& );


string _bin2hex(const char *);
#endif
