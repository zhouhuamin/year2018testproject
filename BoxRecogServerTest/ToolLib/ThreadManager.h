// ThreadManager.h: interface for the CThreadManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _THREAD_MANAGER_H_
#define _THREAD_MANAGER_H_

#include "MRSWInt.h"

class CThreadManager  
{
public:
	CThreadManager(){}
	virtual ~CThreadManager(){CloseAll();}

	void Open()
	{
		CloseAll();

		m_bThreadContinue.Set(true);
		m_nThreadCount.Set(0);

	}

	void CloseAll()
	{
		m_bThreadContinue.Set(false);
		while (m_nThreadCount.Get())
		{
			FreeLongMinSleep();
		}
	}

	int AddAThread()
	{
		return m_nThreadCount.Add();
	}


	int DecAThread()
	{
		return m_nThreadCount.Dec();
	}


	bool ThreadContinue()
	{
		return m_bThreadContinue.Get();
	}

	int GetThreadCount()
	{
		return m_nThreadCount.Get();
	}

	int GetID()
	{
		return m_nThreadID.Add()-1;
	}

private:
	CMRSWInt            m_nThreadCount;
	CMRSWBool           m_bThreadContinue;
	CMRSWInt            m_nThreadID;
};

#endif 
