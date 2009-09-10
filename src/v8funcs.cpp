#include "v8funcs.h"
#include "trackertypes.h"
#include <mysql.h>
#include <iostream>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

using namespace std;
using namespace v8;
MYSQL mysql;


static char hexconvtab[] = "0123456789abcdef";

string _bin2hex(const char *istr) {
    string m;
    int j = 0;
    char re[41];
    for (int i = 0; i < 20; i++) {
        char c = istr[i] & 0xff;
        re[j++] = hexconvtab[ (c >> 4) & 15 ];
        re[j++] = hexconvtab[c & 15];
    }
    re[j] = '\0';
    m = re;
    return m;
}

Handle<String> bin2hex(const char *istr) {
    Handle<String> m = String::New(_bin2hex(istr).c_str());
    return m;
}

Handle<Value> Print(const v8::Arguments& args) {
    bool first = true;
    for (int i = 0; i < args.Length(); i++) {
        v8::HandleScope handle_scope;
        if (first) {
            first = false;
        } else {
            cout << " ";
        }
        v8::String::Utf8Value str(args[i]);
        cout << *str;
    }
    cout << endl;
    return v8::Undefined();
}

/* -- request -- */
Handle<Value> req_getip(Local<String> property, const AccessorInfo &info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    in_addr_t value = static_cast<TorrentRequest*> (ptr)->ip;
    char *ip = inet_ntoa(*(struct in_addr *) & value);
    return String::New(ip);
};

Handle<Value> req_getinfohash(Local<String> property,
        const AccessorInfo &info) {

    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    char *value = static_cast<TorrentRequest*> (ptr)->torrent;

    return String::New(_bin2hex(value).c_str());
}

Handle<Value> req_getagent(Local<String> property,
        const AccessorInfo &info) {

    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    const char *value = static_cast<TorrentRequest*> (ptr)->useragent.c_str();

    return String::New(value);
}



Handle<Value> req_getquery(Local<String> property,
        const AccessorInfo &info) {

    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    const char *value = static_cast<TorrentRequest*> (ptr)->query.c_str();

    return String::New(value);
}

Handle<Value> req_getpeerid(Local<String> property,
        const AccessorInfo &info) {

    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    char *value = static_cast<TorrentRequest*> (ptr)->peerid;

    return String::New(_bin2hex(value).c_str());
}

Handle<Value> req_getleft(Local<String> property,
        const AccessorInfo &info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    unsigned long value = static_cast<TorrentRequest*> (ptr)->left;
    return Integer::New(value);
}

Handle<Value> req_getdownload(Local<String> property,
        const AccessorInfo &info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    unsigned long value = static_cast<TorrentRequest*> (ptr)->download;
    return Integer::New(value);
}

Handle<Value> req_getupload(Local<String> property,
        const AccessorInfo &info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    unsigned long value = static_cast<TorrentRequest*> (ptr)->upload;
    return Integer::New(value);
}

Handle<Value> req_getcorrupt(Local<String> property,
        const AccessorInfo &info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    unsigned long value = static_cast<TorrentRequest*> (ptr)->corrupt;
    return Integer::New(value);
}

Handle<Value> req_getport(Local<String> property,
        const AccessorInfo &info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    int value = static_cast<TorrentRequest*> (ptr)->port;
    return Integer::New(value);
}

Handle<Value> req_getevent(Local<String> property,
        const AccessorInfo &info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    TorrentRequest::Tevent value = static_cast<TorrentRequest*> (ptr)->event;
    switch (value) {
        case TorrentRequest::START :
            return String::New("started");
        case TorrentRequest::STOP :
            return String::New("stopped");
        case TorrentRequest::COMPLETE :
            return String::New("completed");
        case TorrentRequest::UPDATE :
            return String::New("update");
    }
    return v8::Undefined();
}

Handle<ObjectTemplate> makeRequestTemplate() {
    HandleScope handle_scope;
    Handle<ObjectTemplate> result = ObjectTemplate::New();
    result->SetInternalFieldCount(1);
    result->SetAccessor(String::New("infohash"), req_getinfohash);
    result->SetAccessor(String::New("agent"), req_getagent);
    result->SetAccessor(String::New("query"), req_getquery);
    result->SetAccessor(String::New("peerid"), req_getpeerid);
    result->SetAccessor(String::New("left"), req_getleft);
    result->SetAccessor(String::New("download"), req_getdownload);
    result->SetAccessor(String::New("upload"), req_getupload);
    result->SetAccessor(String::New("corrupt"), req_getcorrupt);
    result->SetAccessor(String::New("port"), req_getport);
    result->SetAccessor(String::New("ip"), req_getip);
    result->SetAccessor(String::New("event"), req_getevent);
    return handle_scope.Close(result);
}
/* -- request */

/* -- peer -- */

Handle<Value> peer_getstate(Local<String> property,
        const AccessorInfo &info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    TorrentPeer::Pstate value = static_cast<TorrentPeer*> (ptr)->state;
    switch (value) {
        case TorrentPeer::SEED :
            return String::New("seeder");
        case TorrentPeer::LEECH :
            return String::New("leecher");
    }
    return v8::Undefined();
}
Handle<Value> peer_getpeerid(Local<String> property,
        const AccessorInfo &info) {

    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    char *value = static_cast<TorrentPeer*> (ptr)->id;

    return String::New(_bin2hex(value).c_str());
}

Handle<Value> peer_getip(Local<String> property, const AccessorInfo &info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    in_addr_t value = static_cast<TorrentPeer*> (ptr)->ip;
    char *ip = inet_ntoa(*(struct in_addr *) & value);
    return String::New(ip);
};

Handle<Value> peer_getleft(Local<String> property,
        const AccessorInfo &info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    unsigned long value = static_cast<TorrentPeer*> (ptr)->left;
    return Integer::New(value);
}

Handle<Value> peer_getdownload(Local<String> property,
        const AccessorInfo &info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    unsigned long value = static_cast<TorrentPeer*> (ptr)->download;
    return Integer::New(value);
}

Handle<Value> peer_getupload(Local<String> property,
        const AccessorInfo &info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    unsigned long value = static_cast<TorrentPeer*> (ptr)->upload;
    return Integer::New(value);
}

Handle<Value> peer_getcorrupt(Local<String> property,
        const AccessorInfo &info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    unsigned long value = static_cast<TorrentPeer*> (ptr)->corrupt;
    return Integer::New(value);
}

Handle<Value> peer_getport(Local<String> property,
        const AccessorInfo &info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    int value = static_cast<TorrentPeer*> (ptr)->port;
    return Integer::New(value);
}

Handle<Value> peer_getlastseen(Local<String> property,
        const AccessorInfo &info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    double value = static_cast<TorrentPeer*> (ptr)->lastseen;
    return Date::New(value);
}

Handle<ObjectTemplate> makePeerTemplate() {
    HandleScope handle_scope;
    Handle<ObjectTemplate> result = ObjectTemplate::New();
    result->SetInternalFieldCount(1);
    result->SetAccessor(String::New("left"), peer_getleft);
    result->SetAccessor(String::New("download"), peer_getdownload);
    result->SetAccessor(String::New("upload"), peer_getupload);
    result->SetAccessor(String::New("corrupt"), peer_getcorrupt);
    result->SetAccessor(String::New("port"), peer_getport);
    result->SetAccessor(String::New("ip"), peer_getip);
    result->SetAccessor(String::New("peerid"), peer_getpeerid);
	result->SetAccessor(String::New("state"), peer_getstate);
    result->SetAccessor(String::New("lastseen"), peer_getlastseen);

    return handle_scope.Close(result);
}

/* -- peer -- */

/* -- torrent -- */
Handle<Value> tor_getlastseen(Local<String> property,
        const AccessorInfo &info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    double value = static_cast<TorrentFile*> (ptr)->lastseen;
    return Date::New(value);
}

Handle<Value> tor_getinfohash(Local<String> property,
        const AccessorInfo &info) {

    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    char *value = static_cast<TorrentFile*> (ptr)->id;

    return String::New(_bin2hex(value).c_str());
}

Handle<Value> tor_gethosts(Local<String> property,
        const AccessorInfo &info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    unsigned long value = static_cast<TorrentFile*> (ptr)->hosts;
    return Integer::New(value);
}

Handle<Value> tor_getseeders(Local<String> property,
        const AccessorInfo &info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    unsigned long value = static_cast<TorrentFile*> (ptr)->seeders;
    return Integer::New(value);
}

Handle<Value> tor_getdownloads(Local<String> property,
        const AccessorInfo &info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    unsigned long value = static_cast<TorrentFile*> (ptr)->downloads;
    return Integer::New(value);
}

Handle<Value> tor_getleechers(Local<String> property,
        const AccessorInfo &info) {
    Local<Object> self = info.Holder();
    Local<External> wrap = Local<External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    unsigned long lvalue = static_cast<TorrentFile*> (ptr)->hosts;
    unsigned long rvalue = static_cast<TorrentFile*> (ptr)->seeders;
    return Integer::New(lvalue - rvalue);
}

Handle<ObjectTemplate> makeTorrentTemplate() {
    HandleScope handle_scope;
    Handle<ObjectTemplate> result = ObjectTemplate::New();
    result->SetInternalFieldCount(1);
    result->SetAccessor(String::New("infohash"), tor_getinfohash);
    result->SetAccessor(String::New("hosts"), tor_gethosts);
    result->SetAccessor(String::New("seeders"), tor_getseeders);
    result->SetAccessor(String::New("leechers"), tor_getleechers);
    result->SetAccessor(String::New("downloads"), tor_getdownloads);
    result->SetAccessor(String::New("lastseen"), tor_getlastseen);
    return handle_scope.Close(result);
}



/* -- torrent -- */

/* mysql */
Handle<Value> mysqlQuery(const v8::Arguments& args) {
    MYSQL_RES *res;
    MYSQL_ROW row;
    MYSQL_FIELD *fields;
    char *k;
    char *v;
    int nfields;
    Handle<Value> retb;

    if (args.Length() != 1) {
        retb = v8::Undefined();
        return retb;
    }
    v8::String::Utf8Value str(args[0]);
    const char* cstr = *str;

    if (mysql_real_query(&mysql, cstr, str.length())) {
        return v8::Boolean::New(false);
    }

    res = mysql_use_result(&mysql);
    if (res == NULL) {
        return v8::Undefined();
    }

    nfields = mysql_num_fields(res);
    fields = mysql_fetch_fields(res);

    if ((row = mysql_fetch_row(res))) {
        Handle<Object> retf = Object::New();
        for (int i = 0; i < nfields; i++) {
            k = fields[i].name;
            v = row[i];
            retf->Set(String::New(k), String::New(v));
        }
        retb = retf;
    } else {
        retb = v8::Undefined();
    }
    mysql_free_result(res);
    return retb;
}

Handle<Value> mysqlEscape(const v8::Arguments& args) {
    if (args.Length() != 1) {
        return v8::Boolean::New(false);
    }

    v8::String::Utf8Value str(args[0]);
    char b[str.length() * 2 + 1];
    unsigned long ret = 0;
    ret = mysql_real_escape_string(&mysql, b, *str, str.length());
    return v8::String::New(b, ret);
}

Handle<Value> mysqlConnect(const v8::Arguments& args) {
    if (args.Length() != 4) {
        return v8::Boolean::New(false);
    }

    v8::String::Utf8Value host(args[0]);
    v8::String::Utf8Value user(args[1]);
    v8::String::Utf8Value pw(args[2]);
    v8::String::Utf8Value db(args[3]);
    //initiate mysql
    mysql_init(&mysql);
    if (mysql_real_connect(&mysql, *host, *user, *pw, *db, MYSQL_PORT, NULL, 0) == NULL) {
        cout << "mysqlConnect failed: " << mysql_error(&mysql) << endl;
        return v8::Boolean::New(false);
    }
    char a0 = true;
    mysql_options(&mysql, MYSQL_OPT_RECONNECT, &a0);

    return v8::Boolean::New(true);
}

/* mysql */


Handle<ObjectTemplate> makeFuncs() {
    HandleScope handle_scope;

    Handle<ObjectTemplate> result = ObjectTemplate::New();
    result->Set(String::New("print"), FunctionTemplate::New(Print));
    result->Set(String::New("mysqlQuery"), FunctionTemplate::New(mysqlQuery));
    result->Set(String::New("mysqlConnect"), FunctionTemplate::New(mysqlConnect));
    result->Set(String::New("mysqlEscape"), FunctionTemplate::New(mysqlEscape));
    //result->Set(String::New("bin2hex"), FunctionTemplate::New(bin2hex));
    return handle_scope.Close(result);
}



// Reads a file into a v8 string.

v8::Handle<v8::String> ReadFile(const char* name) {
    FILE* file = fopen(name, "rb");
    if (file == NULL) return v8::Handle<v8::String > ();

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
    v8::Handle<v8::String> result = v8::String::New(chars, size);
    delete[] chars;
    return result;
}

