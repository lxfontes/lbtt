#ifndef __THREAD_H__
#define __THREAD_H__
#include <pthread.h>
#include <functional>
using namespace std;
template<class T>
struct fthread{
	fthread(){}
	static void *run(void *x){ T *i_ = (T *)x; i_->run();return NULL;}
};

template<class T>
struct thread{
	thread(T *x) { pthread_create(&tid,NULL,fthread<T>::run,x);}
	protected:
	pthread_t tid;
	fthread<T> x_;
};

#endif

