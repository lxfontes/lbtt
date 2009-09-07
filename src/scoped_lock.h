/* 
 * File:   scoped_lock.h
 * Author: lfontes
 *
 * Created on September 4, 2009, 5:23 PM
 */

#ifndef _SCOPED_LOCK_H
#define	_SCOPED_LOCK_H


#include <pthread.h>

class scoped_lock {
    pthread_mutex_t *m;
public:

    explicit scoped_lock(pthread_mutex_t &m_) : m(&m_) {
        pthread_mutex_lock(m)
                ;
    }

    ~scoped_lock() {
        pthread_mutex_unlock(m);
    }
};



#endif	/* _SCOPED_LOCK_H */

