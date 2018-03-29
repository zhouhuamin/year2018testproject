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
 *	����Ϊ���ʱ�����û�н�������״̬�����ã������棬����Ѿ����ý����־�������ã����ؼ�
 *  ����Ϊ�ٵ�ʱ�����ǳɹ�������
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
				//ԭʼֵ��false,��ʾ��������
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
			m_bAlreadRunFlag=false;     //����������Ϊfalse
			bRet=true;
		}
		m_Lock.Unlock();

	}

	return bRet;
}
