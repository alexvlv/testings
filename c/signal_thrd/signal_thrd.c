/*
$Id$
*/

#define NUM_THRDS 10

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>
#include <signal.h>


#include "sig.h"
#include "trace.h"
#include ".git.h"
#ifndef VERSION
#define VERSION " "
#endif

DBG_DEFINE_TRACE_ALL();


static pthread_t tid[NUM_THRDS];
//-------------------------------------------------------------------------
static void forever(int id)
{
    int rc = 0;
    while(1) {
		rc = sleep(60);
		if(rc != 0) TRACE("#%d: sleep interrupted: %d, tid:%d",id,rc,get_tid());
	}
}
//-------------------------------------------------------------------------
static void* thread_func(void *args) {
	int id = (intptr_t)args;
	// int id = *((int*)(&arg));
	DBG("Thread #%d started, tid:%d",id,get_tid());
	if(id == (NUM_THRDS-1) ) {
		DBG("Thread #%d, tid:%d - ignore SIGHUP!",id,get_tid());
		// Run test: ignored in all threads !!!!
		signal(SIGHUP, SIG_IGN);
	}
    forever(id);
    return NULL;
}
//-------------------------------------------------------------------------
int main(void)
{
	INFO("Signal thread handling, GIT: " VERSION	" [ Compiled: " __DATE__ " " __TIME__ " ](pid %d)*",getpid());
	
	int rc, i = 0;
	intptr_t idx;
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	signal_setup();
	
	for(i=0;i<NUM_THRDS;i++) {
		idx = i;
		rc = pthread_create(&tid[i],&attr,thread_func,(void *)idx);
		if(rc != 0 ) {
			WARNING("pthread_create failed(%d)",rc);
			break;
		}
	}
	forever(100);
	return 0;
}
//-------------------------------------------------------------------------

/*
Run on Xubuntu 18.04
1. Handler called directly in tid(LWP) id
2. If signal blocked in one thread, it's blocked in each thread
man 2 signal:
The effects of signal() in a multithreaded process are unspecified.
!!!!!


ps -eLf | grep signal_


https://stackoverflow.com/questions/1640423/error-cast-from-void-to-int-loses-precision

ToDo:
https://devarea.com/linux-handling-signals-in-a-multithreaded-application/
According to the POSIX standard all threads should appear with 
the same PID on the system and using pthread_sigmask() 
you can define the signal blocking mask for every thread.
https://stackoverflow.com/questions/2575106/posix-threads-and-signals
Use pthread_sigmask
https://stackoverflow.com/questions/23225211/example-of-handling-signals-in-multi-threaded-process

*/
