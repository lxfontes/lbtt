/* 
 * File:   thread.h
 * Author: lfontes
 *
 * Created on September 4, 2009, 4:52 PM
 */

#ifndef _THREAD_H
#define	_THREAD_H

#include <pthread.h>
#include <functional>
using namespace std;



template<class K>
struct thread{
        thread(K &x):r(x) { pthread_create(&tid,NULL,thread<K>::run,this);}
        static void *run(void *x){
            static_cast<thread<K> *>(x)->runme();
            return NULL;
        }
        void *runme(){
            //r.method();
            r();
            return NULL;
        }
        ~thread(){ pthread_join(tid,NULL); }
        protected:
        pthread_t tid;
        K &r;
};



#endif	/* _THREAD_H */

