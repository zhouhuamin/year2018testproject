// NonReentrantLock.h: interface for the CNonReentrantLock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined _NON_REENTRANT_LOCK_H_
#define _NON_REENTRANT_LOCK_H_

#include "MutexLock.h"
class CNonReentrantLock  
{
public:
	CNonReentrantLock();
	~CNonReentrantLock();

public:
	bool Set(bool bRunFlag);
private:
	CMutexLock           m_Lock;
	bool                 m_bAlreadRunFlag;

};

#endif 
