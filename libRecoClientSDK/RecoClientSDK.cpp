#include "RecoClientSDK.h"

#include <stdio.h>
#include <string.h>

#include "ControlCmdHandler.h"

CControlCmdHandler* g_pCtrlCmdhandler=NULL;


int RecoInit(char* szRecoServerIP,int nRecoServerPort)
{
    if(!g_pCtrlCmdhandler)
    {
        g_pCtrlCmdhandler=new CControlCmdHandler;
        g_pCtrlCmdhandler->Init(szRecoServerIP,nRecoServerPort);
    }
    
    return 0;
}

int  ContaReco(char* szRecoSequence,char *ImagePath)
{
    if(g_pCtrlCmdhandler)
    {
        g_pCtrlCmdhandler->RecoConta(szRecoSequence, ImagePath);

    }
    return 0;
    
}


int Release()
{
    if(g_pCtrlCmdhandler)
    {
        g_pCtrlCmdhandler->Stop();
        delete g_pCtrlCmdhandler;
        g_pCtrlCmdhandler=NULL;
    }
    
    return 0;
}


int SetRecoResultCallback(_RECO_RESULT_CALLBACK pRecoResultCallback)
{
    if(g_pCtrlCmdhandler)
    {
        g_pCtrlCmdhandler->SetRecoResultCallback(pRecoResultCallback);
    }
    
    return 0;
}