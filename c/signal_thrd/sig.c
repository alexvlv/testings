/**************************************************************************
$Id$
Based on: http://server/svn/077_Codecs/Dec-DaVinci-BF/trunk/Firmware/ARM/soft/mjpeg/sig.c
LastChangedRevision: 1089
LastChangedDate: 2017-12-19 16:45:32 +0300 (Tue, 19 Dec 2017)
LastChangedBy: alexvol
***************************************************************************/


#include "dbg.h"
#include "macro.h"

#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

//-------------------------------------------------------------------------
static void signal_handler(int signal)
{
	const char *desc = strsignal(signal);
	switch(signal) {
		case SIGINT:
		case SIGQUIT:
		case SIGTERM:
			ERR("Signal %i[%s], pid:%d/%d, exit!", signal,desc,getpid(),get_tid());
			exit(1);
			break;
		case SIGABRT:
			ERR("Signal %i[%s], pid:%d/%d, aborted!", signal,desc,getpid(),get_tid());
			exit(1);
			break;
		case SIGBUS:
		case SIGFPE:
		case SIGILL:
		case SIGSEGV:
		case SIGSYS:
			ERR("Signal %i[%s], pid:%d/%d, terminated!", signal,desc,getpid(),get_tid());
			exit(1);
			break;
		//case SIGPIPE:
		//case SIGCHLD:
		//	break;
		default:
			DBG("Signal %i[%s], pid:%d/%d, ignored", signal,desc,getpid(),get_tid());
	}
	return;
}
//-------------------------------------------------------------------------
void signal_setup_listed(void)
{
	unsigned ch;
	int signals[] = {SIGHUP,SIGINT,SIGQUIT,SIGILL,SIGABRT,SIGFPE,SIGPIPE,SIGPIPE,SIGBUS,SIGALRM,SIGTERM};
	for(ch=0; ch<ARRAY_SIZE(signals); ch++) signal(signals[ch],signal_handler);
	errno=0;
}
//-------------------------------------------------------------------------
void signal_setup(void)
{
	int ch;
	for(ch=1; ch<28; ch++) signal(ch,signal_handler);
	errno=0;
}
//-------------------------------------------------------------------------
