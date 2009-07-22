#ifndef __SFCGI_H__
#define __SFCGI_H__
#include "fcgio.h"
#include "tracker.h"
#include <boost/thread.hpp>
class sfcgi
{
public:
        sfcgi (tracker &t,int port){ tracker_ = &t; string p(":"); p += boost::lexical_cast<std::string>(port);
        FCGX_Init();
        usock = FCGX_OpenSocket(p.c_str(),10); 
        boost::thread thr(boost::bind(&sfcgi::run,this));
    }
    	static int decode_q(const char *uri,char *ret){
    		        char c;
        int i, j, in_query = 0;

        for (i = j = 0; uri[i] != '\0' && uri[i]!='&' && j < 20; i++) {
                c = uri[i];
                if (c == '?') {
                        in_query = 1;
                } else if (c == '+' && in_query) {
                        c = ' ';
                } else if (c == '%' && isxdigit(uri[i+1]) &&
                    isxdigit(uri[i+2])) {
                        char tmp[] = { uri[i+1], uri[i+2], '\0' };
                        c = (char)strtol(tmp, NULL, 16);
                        i += 2;
                }
                ret[j++] = c;
        }
        ret[j]='\0';

        return j;
    	}
        void run();
private:
        int usock;
        tracker *tracker_;
};

#endif
