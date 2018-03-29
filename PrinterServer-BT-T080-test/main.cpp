#include "includes.h"
#include "packet_format.h"
#include "pthread.h"
#include "starup_handle.h"
#include "port.h"
#include <cstdlib>
#include "ace/Signal.h"
#include "ace/streams.h"
#include "ace/Thread_Manager.h"
#include "ace/Select_Reactor.h"
#include "MSGHandleCenter.h"

#include "SysUtil.h"

#include <syslog.h>
#include <dlfcn.h>
#include <stack>
#include <vector>
#include <new>

pthread_t tid_DEV;
pthread_t tid_worker_task;
pthread_t tid_net;

pthread_t tid_arrived;
pthread_t tid_started;
pthread_t tid_stopped;
pthread_t tid_timer;
pthread_t tid_DEV_test;

volatile u32 DEV_request_count = 0;
volatile int gFd_dev = -1;

using namespace std;

static ACE_THR_FUNC_RETURN event_loop(void *arg)
{
    ACE_Reactor *reactor = (ACE_Reactor*)arg;
    reactor->owner(ACE_OS::thr_self());
    reactor->run_reactor_event_loop();
    return 0;
}

int main(int argc, char** argv)
{
    ACE_Sig_Action sig((ACE_SignalHandler) SIG_IGN, SIGPIPE);
    ACE_UNUSED_ARG(sig);

    int server_fd;
    openlog("PrinterServer", LOG_PID, LOG_LOCAL4);

    param_init_handle();

    ACE_Select_Reactor *select_reactor;
    ACE_NEW_RETURN(select_reactor, ACE_Select_Reactor, 1);
    ACE_Reactor *reactor;
    ACE_NEW_RETURN(reactor, ACE_Reactor(select_reactor, 1), 1);
    ACE_Reactor::close_singleton();
    ACE_Reactor::instance(reactor, 1);

    ACE_Thread_Manager::instance()->spawn(event_loop, reactor);
	syslog(LOG_DEBUG, "starting up ledserver daemon\n");
	syslog(LOG_DEBUG, "starting up reactor event loop ...\n");
    MSG_HANDLE_CENTER::instance()->open();

    ACE_Reactor* reactorptr = ACE_Reactor::instance();

    pthread_create(&tid_DEV, NULL, pthread_dev_handle2, NULL);
    syslog(LOG_DEBUG, "create the DEV pthread OK!\n");
    
    pthread_create(&tid_DEV_test, NULL, pthread_dev_handle_test, NULL);
    syslog(LOG_DEBUG, "create the test pthread OK!\n");       
    

    MSG_HANDLE_CENTER::instance()->wait();
    ACE_Thread_Manager::instance()->wait();
    ACE_DEBUG((LM_DEBUG, "(%P|%t) shutting down ledserver daemon\n"));

    close(gFd_dev);

    closelog();
    usleep(50 * 1000);
    return 0;
}

