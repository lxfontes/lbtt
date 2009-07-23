#ifndef __SCOPE_LOCK_H__
#define __SCOPE_LOCK_H__

#include <pthread.h>
class scoped_lock{
	pthread_mutex_t *m;
	public:
	explicit scoped_lock(pthread_mutex_t &m_):m(&m_) { pthread_mutex_lock(m); }
	~scoped_lock(){ pthread_mutex_unlock(m);}
};
#endif

