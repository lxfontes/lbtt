/* 
 * File:   v8funcs.h
 * Author: lfontes
 *
 * Created on September 6, 2009, 5:14 PM
 */

#ifndef _V8FUNCS_H
#define	_V8FUNCS_H
#include <v8.h>


#include <iostream>
#include <string>


using namespace v8;
using namespace std;

Handle<ObjectTemplate> makeFuncs();
Handle<ObjectTemplate> makeRequestTemplate();
Handle<ObjectTemplate> makePeerTemplate();
Handle<ObjectTemplate> makeTorrentTemplate();
Handle<String> ReadFile(const char* name);
#endif	/* _V8FUNCS_H */

