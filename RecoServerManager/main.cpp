#include "ace/Signal.h"
#include "SimpleConfig.h"
#include "ace/streams.h"
#include "ace/Thread_Manager.h"
#include "ace/Select_Reactor.h"
#include "MyLog.h"
#include "MSG_Center.h"
#include "Cmd_Acceptor.h"

static ACE_THR_FUNC_RETURN event_loop(void *arg)
{
    ACE_Reactor *reactor = (ACE_Reactor *) arg;
    reactor->owner(ACE_OS::thr_self());
    reactor->run_reactor_event_loop();
    return 0;
}

int main(int argc, char *argv[])
//int rsmain()
{

    /* Ignore signals generated when a connection is broken unexpectedly. */

    ACE_Sig_Action sig((ACE_SignalHandler) SIG_IGN, SIGPIPE);
    ACE_UNUSED_ARG(sig);


    //ACE_UNUSED_ARG(argc);
    //ACE_UNUSED_ARG(argv);

    CMyLog::Init();
    CSimpleConfig cg;
    cg.get_config();



    //////////////////////////////////////////////////////////////////////////////////////////////
    //do timeout event

    /* get default instance of ACE_Reactor  */
    ACE_Select_Reactor *select_reactor;
    ACE_NEW_RETURN(select_reactor, ACE_Select_Reactor, 1);
    ACE_Reactor *reactor;
    ACE_NEW_RETURN(reactor, ACE_Reactor(select_reactor, 1), 1);
    ACE_Reactor::close_singleton();
    ACE_Reactor::instance(reactor, 1);


    ACE_DEBUG((LM_DEBUG, "(%P|%t) starting up controlcenter daemon\n"));
    ACE_DEBUG((LM_DEBUG, "(%P|%t) starting up reactor event loop ...\n"));

    /*
       struct timeval tv;
       tv.tv_sec = 1;
       tv.tv_usec = 0;
       select(0, 0, NULL, NULL, &tv);
     */
    MSG_CENTER::instance()->open();


    CCmd_Acceptor::InitHandler();
    CCmd_Acceptor peer_acceptor;
    if (peer_acceptor.open(ACE_INET_Addr(CSimpleConfig::RECO_LISTEN_PORT), reactor, 1) == -1)
    {
        ACE_ERROR_RETURN((LM_ERROR, "%p\n", "open listen port fail..."), -1);
    }
    else
    {
        ACE_DEBUG((LM_DEBUG, "(%P|%t) open listen port %d ok...\n", CSimpleConfig::RECO_LISTEN_PORT));
    }


 
    //	ACE_Thread_Manager::instance()->spawn(proactor_event_loop, r);
    ACE_DEBUG((LM_DEBUG, "(%P|%t) starting reco server manager daemon\n"));

    ACE_Thread_Manager::instance()->spawn(event_loop, reactor);
    MSG_CENTER::instance()->wait();
    ACE_Thread_Manager::instance()->wait();

    ACE_DEBUG((LM_DEBUG, "(%P|%t) shutting down reco server manager daemon\n"));

    return 0;
}

int Daemon()
{
	pid_t pid;
	pid = fork();
	if (pid < 0)
	{
		return -1;
	}
	else if (pid != 0)
	{
		exit(0);
	}
	setsid();
	return 0;
}
#define SOFTWARE_VERSION  "version 1.0.0.0"         //����汾��

//int main(int argc, char *argv[])
//{
//	int iRet;
//	int iStatus;
//	pid_t pid;
//	//��ʾ�汾��
//	if (argc == 2)
//	{
//		//����ǲ鿴�汾��
//		if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "-V") || !strcmp(argv[1], "-Version") || !strcmp(argv[1], "-version"))
//		{
//			printf("%s  %s\n", argv[0], SOFTWARE_VERSION);
//			return 0;
//		}
//	}
//
//	Daemon();
//
//createchildprocess:
//	//��ʼ�����ӽ���
//	printf("begin to create	the child process of %s\n", argv[0]);
//
//	int itest = 0;
//	switch (fork())//switch(fork())
//	{
//	case -1 : //�����ӽ���ʧ��
//		printf("cs�����ӽ���ʧ��\n");
//		return -1;
//	case 0://�ӽ���
//		printf("cs�����ӽ��̳ɹ�\n");
//		rsmain();
//		return -1;
//	default://������
//		pid = wait(&iStatus);
//		printf("�ӽ����˳���5�����������......\n");
//		sleep(5);
//		goto createchildprocess; //������������
//		break;
//	}
//	return 0;
//}
