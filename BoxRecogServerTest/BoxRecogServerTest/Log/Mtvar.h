#if !defined _MULTI_THREAD_VAR_H_
#define _MULTI_THREAD_VAR_H_

#include "MutexLock.h"

typedef struct _MINT_            //整型的多线程安全单元
{
	int      m_nValue;
	MUTEX    m_MyLock;
}MINT,MBOOL;

int inline MvarInit(MINT& mValue,int nValue)
{
	MUTEXINIT(&mValue.m_MyLock);
	mValue.m_nValue=nValue;
	return nValue;
}


void inline MvarDestroy(MINT& mValue)
{
	MUTEXLOCK(&mValue.m_MyLock);
	MUTEXUNLOCK(&mValue.m_MyLock);
	MUTEXDESTROY(&mValue.m_MyLock);
}

int inline MvarSet(MINT& mValue,int nValue)
{
	MUTEXLOCK(&mValue.m_MyLock);
	mValue.m_nValue=nValue;
	MUTEXUNLOCK(&mValue.m_MyLock);
	return nValue;
}

int inline MvarGet(MINT& mValue)
{
	int nValue=0;
	MUTEXLOCK(&mValue.m_MyLock);
	nValue=mValue.m_nValue;
	MUTEXUNLOCK(&mValue.m_MyLock);
	return nValue;
}

int inline MvarADD(MINT& mValue,int nValue=1)
{
	int nRet=0;
	MUTEXLOCK(&mValue.m_MyLock);
	mValue.m_nValue+=nValue;
	nRet=mValue.m_nValue;
	MUTEXUNLOCK(&mValue.m_MyLock);

	return nRet;
}

int inline MvarDEC(MINT& mValue,int nValue=1)
{
	int nRet=0;
	MUTEXLOCK(&mValue.m_MyLock);
	mValue.m_nValue-=nValue;
	nRet=mValue.m_nValue;
	MUTEXUNLOCK(&mValue.m_MyLock);
	
	return nRet;
}


#define XMI MvarInit
#define XME MvarDestroy
#define XMG MvarGet
#define XMS MvarSet
#define XMA MvarADD
#define XMD MvarDEC

#endif