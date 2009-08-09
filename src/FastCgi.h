#ifndef __SFCGI_H__
#define __SFCGI_H__
#include <iostream>
#include <sstream>
#include "fcgio.h"
#include "thread.hpp"
#include "Tracker.h"

class FastCgi
{
public:
	FastCgi(Tracker &,const char *);
	void run();
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
	
private:
	int usock;
	Tracker &m_tracker;

};

#endif
