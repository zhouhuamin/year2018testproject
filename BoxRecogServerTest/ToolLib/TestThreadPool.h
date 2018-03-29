// TestThreadPool.h: interface for the CTestThreadPool class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TESTTHREADPOOL_H__B60E92A1_E393_4BDD_92F5_739BD2ED7280__INCLUDED_)
#define AFX_TESTTHREADPOOL_H__B60E92A1_E393_4BDD_92F5_739BD2ED7280__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "FreeLongThreadPool.h"



class CTestThreadPool  
{
public:
	CTestThreadPool();
	virtual ~CTestThreadPool();

	static void TestCallBack(void* pCallparam,MBOOL& bThreadContinue);
	static bool TestTaskCallBack(void* pCallParam, int& nStatus);

	int m_nTestNo;
};

#endif // !defined(AFX_TESTTHREADPOOL_H__B60E92A1_E393_4BDD_92F5_739BD2ED7280__INCLUDED_)
