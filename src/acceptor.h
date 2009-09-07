/* 
 * File:   Acceptor.h
 * Author: lfontes
 *
 * Created on September 4, 2009, 4:37 PM
 */

#ifndef _ACCEPTOR_H
#define	_ACCEPTOR_H

#include "tracker.h"
#include <ev++.h>

class Acceptor{
public:
    Acceptor(const char * ip,int port);
    void setCallback(void (*cb)(int));
    void operator()();
    void stop(){ loop.unloop(ev::ALL);}
private:
        
        void newClient(ev::io &,int);
        ev::dynamic_loop loop; //our main loop
        ev::io iow;
        void (*m_cb)(int);
};

#endif	/* _ACCEPTOR_H */

