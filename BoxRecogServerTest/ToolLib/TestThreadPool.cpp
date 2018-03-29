// TestThreadPool.cpp: implementation of the CTestThreadPool class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TestThreadPool.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTestThreadPool::CTestThreadPool()
{
	m_nTestNo=100;
}

CTestThreadPool::~CTestThreadPool()
{

}

void CTestThreadPool::TestCallBack(void* pCallparam,MBOOL& bThreadContinue)
{
	CTestThreadPool* pThreadPool=(CTestThreadPool*)pCallparam;
	int mmm=0;
	for(int i=0;i<9;i++)
	{
		printf("this is message %d \n",i);
		Sleep(1000);
	}
}

bool CTestThreadPool::TestTaskCallBack(void* pCallParam, int& nStatus)
{
	CTestThreadPool* pThreadPool=(CTestThreadPool*)pCallParam;
	for(int i=0;i<5;i++)
	{
		printf("this is message %d \n",i);
		Sleep(1000);
	}

	return true;
}
