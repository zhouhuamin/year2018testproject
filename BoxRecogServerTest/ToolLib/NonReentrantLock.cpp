// NonReentrantLock.cpp: implementation of the CNonReentrantLock class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NonReentrantLock.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNonReentrantLock::CNonReentrantLock()
{
	m_bAlreadRunFlag=false;
}

CNonReentrantLock::~CNonReentrantLock()
{

}

/*
 *	设置为真的时候，如果没有进入设置状态，设置，返回真，如果已经设置进入标志，不设置，返回假
 *  设置为假的时候，总是成功返回真
 */
bool CNonReentrantLock::Set(bool bRunFlag)
{
	bool bRet=false;
	if(bRunFlag)
	{
		m_Lock.Lock();
		{
			if(!m_bAlreadRunFlag)
			{
				//原始值是false,表示可以设置
				m_bAlreadRunFlag=true;
				bRet=true;
			}
		}
		m_Lock.Unlock();
	}
	else
	{
		m_Lock.Lock();
		{
			m_bAlreadRunFlag=false;     //无条件设置为false
			bRet=true;
		}
		m_Lock.Unlock();

	}

	return bRet;
}
