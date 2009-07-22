#ifndef __HTTP_H__
#define __HTTP_H__
#include <sys/types.h>
#include "tracker.h"
class processor{
public:
	processor(tracker &t):m_tracker(t){}
	int process(int sockfd,char *inb,size_t ins,char *outb,size_t outs);
protected:
	tracker &m_tracker;
	int dumperr(const char *err,char *outb,size_t outs);
	int decode_q(const char *,char *);
};

#endif
