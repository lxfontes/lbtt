#include "v8funcs.h"
#include "Tracker.h"
#include <mysql.h>
#include <iostream>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

using namespace v8;
using namespace std;

MYSQL mysql;

static char hexconvtab[] = "0123456789abcdef";

string _bin2hex(const char *istr){
	string m;
	int j = 0;
	char re[41];
	for(int i=0;i<20;i++){
		char c = istr[i] & 0xff;
		re[j++]=hexconvtab[ (c >> 4) & 15 ];
		re[j++]=hexconvtab[c & 15];
	}
	re[j]='\0';
	m = re;
	return m;
}

Handle<String> bin2hex(const char *istr){
	Handle<String> m = String::New(_bin2hex(istr).c_str());
	return m;
}


Handle<Object> wrapRequest(Request &req){
	Handle<Object> retf = Object::New();
	char *ip = inet_ntoa(*(struct in_addr *)&req.ip);
	retf->Set(String::New("left"),Number::New(req.left));
	retf->Set(String::New("corrupt"),Number::New(req.corrupt));
	retf->Set(String::New("downloaded"),Number::New(req.download));
	retf->Set(String::New("uploaded"),Number::New(req.upload));
	retf->Set(String::New("numwant"),Number::New(req.numwant));
	retf->Set(String::New("port"),Number::New(req.port));
	retf->Set(String::New("passkey"),String::New(req.pass));
	retf->Set(String::New("_infohash"),String::New(req.torrent,20));
	retf->Set(String::New("hexhash"),bin2hex(req.torrent));
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
Handle<Object> wrapTorrent(Torrent *tor){
	Handle<Object> rett = Object::New();
	rett->Set(String::New("complete"),Number::New(tor->seeders));
	rett->Set(String::New("incomplete"),Number::New(tor->hosts - tor->seeders));
	rett->Set(String::New("download"),Number::New(tor->download));
	rett->Set(String::New("lastseen"),Date::New(tor->lastseen));
	rett->Set(String::New("_infohash"),String::New(tor->id,20));
	rett->Set(String::New("hexhash"),bin2hex(tor->id));
	return rett;
}

Handle<Object> wrapPeer(Peer *pee){
	Handle<Object> rett = Object::New();
	char *ip = inet_ntoa(*(struct in_addr *)&pee->ip);
	rett->Set(String::New("left"),Number::New(pee->left));
	rett->Set(String::New("downloaded"),Number::New(pee->download));
	rett->Set(String::New("uploaded"),Number::New(pee->upload));
	rett->Set(String::New("corrupt"),Number::New(pee->corrupt));
	rett->Set(String::New("lastseen"),Date::New(pee->lastseen));
	rett->Set(String::New("passkey"),String::New(pee->pass.c_str()));
	rett->Set(String::New("_peerid"),String::New(pee->id,20));
	rett->Set(String::New("hexpeerid"),bin2hex(pee->id));
	rett->Set(String::New("ip"),String::New(ip));
	rett->Set(String::New("port"),Number::New(pee->port));
	return rett;
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




Handle<Value> Print(const v8::Arguments& args){
	bool first = true;
	for (int i = 0; i < args.Length(); i++) {
		v8::HandleScope handle_scope;
		if (first) {
			first = false;
		} else {
			cout << " ";
		}
		v8::String::Utf8Value str(args[i]);
		cout << *str ;
	}
	cout << endl;
	return v8::Undefined();
}



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
	const char* cstr = *str;

	if(mysql_real_query(&mysql,cstr,str.length())){
		return v8::Boolean::New(false);
	}

	res = mysql_use_result(&mysql);
	if(res == NULL){
		return v8::Undefined();
	}

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

Handle<Value> mysqlEscape(const v8::Arguments& args){
	if(args.Length() != 1){
		return v8::Boolean::New(false);
	}

	v8::String::Utf8Value str(args[0]);
	char b[str.length() * 2 + 1];
	unsigned long ret = 0;
	ret = mysql_real_escape_string(&mysql,b,*str,str.length());
	return v8::String::New(b,ret);
}


Handle<Value> mysqlConnect(const v8::Arguments& args){
	if(args.Length() != 4){
		return v8::Boolean::New(false);
	}

	v8::String::Utf8Value host(args[0]);
	v8::String::Utf8Value user(args[1]);
	v8::String::Utf8Value pw(args[2]);
	v8::String::Utf8Value db(args[3]);
	//initiate mysql
	mysql_init(&mysql);
	if(mysql_real_connect(&mysql, *host,*user,*pw,*db, MYSQL_PORT, NULL, 0) == NULL){
		cout << "mysqlConnect failed: " << mysql_error(&mysql) << endl;
		return v8::Boolean::New(false);
	}
	char a0 = true;
	mysql_options(&mysql, MYSQL_OPT_RECONNECT, &a0);

	return v8::Boolean::New(true);
}

Handle<ObjectTemplate> makeFuncs() {
	HandleScope handle_scope;

	Handle<ObjectTemplate> result = ObjectTemplate::New();
	result->Set(String::New("print"), FunctionTemplate::New(Print));
	result->Set(String::New("mysqlQuery"), FunctionTemplate::New(mysqlQuery));
	result->Set(String::New("mysqlConnect"), FunctionTemplate::New(mysqlConnect));
	result->Set(String::New("mysqlEscape"), FunctionTemplate::New(mysqlEscape));
	// result->Set(String::New("bin2hex"), FunctionTemplate::New(bin2hex));
	// Again, return the result through the current handle scope.
	return handle_scope.Close(result);
}
