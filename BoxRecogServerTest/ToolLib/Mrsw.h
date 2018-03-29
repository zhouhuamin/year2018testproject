#if !defined _MULTI_READ_SINGLE_WRITE_H_
#define _MULTI_READ_SINGLE_WRITE_H_
#include "MutexLock.h"
#include "SysInclude.h"
#include <unistd.h>

typedef struct _FREE_LONG_MULTI_READ_SINGLE_WRITE_LOCK_
{
	int         m_nReadCount;             //��������
	bool        m_bWriteFlag;             //д��־
	MUTEX       m_Lock;                   //��д��
}FreeLongMultiReadSingleWriteLock;

const unsigned long FreeLongMultiReadSingleWriteLockSize=sizeof(FreeLongMultiReadSingleWriteLock);


inline void FreeLongMinSleep()
{
#ifdef WIN32
	Sleep(10);
#else
	struct timespec slptm;
	slptm.tv_sec=0;
	slptm.tv_nsec=1000;
	if(nanosleep(&slptm,NULL)==-1)
	{
		usleep(1);
	}
#endif
}

void inline MRSWLock_Create(FreeLongMultiReadSingleWriteLock* pLock)
{
	MUTEXINIT(&(pLock->m_Lock));              //��ʼ���ڲ���
	pLock->m_nReadCount=0;
	pLock->m_bWriteFlag=false;
}


void inline MRSWLock_Destroy(FreeLongMultiReadSingleWriteLock* pLock)
{
	MUTEXLOCK(&(pLock->m_Lock));
	MUTEXUNLOCK(&(pLock->m_Lock));
	MUTEXDESTROY(&(pLock->m_Lock));
}

bool inline MRSWLock_GetWrites(FreeLongMultiReadSingleWriteLock* pLock)
{
	bool bRet=false;
	MUTEXLOCK(&(pLock->m_Lock));
	{
		bRet=pLock->m_bWriteFlag;
	}
	MUTEXUNLOCK(&(pLock->m_Lock));
	return bRet;
}


int inline MRSWLock_GetRead(FreeLongMultiReadSingleWriteLock* pLock)
{
	int  nRet=0;
	MUTEXLOCK(&(pLock->m_Lock));
	{
		nRet=pLock->m_nReadCount;
	}
	MUTEXUNLOCK(&(pLock->m_Lock));
	return nRet;
}


void inline MRSWLock_EnableWrite(FreeLongMultiReadSingleWriteLock* pLock)
{
	while(1)                   //��ѭ���ȴ�
	{
		MUTEXLOCK(&(pLock->m_Lock));
		{
			if(!pLock->m_bWriteFlag)
			{
				//��������д��־Ϊ��
				pLock->m_bWriteFlag=true;

				//�����˳�
				MUTEXUNLOCK(&(pLock->m_Lock));

				//����goto��ȷ��λ
				goto MRSWLock_EnableWrite_Wait_Read_Clean;
			}
		}

		//���ͷ��ڲ����ٵȴ�
		MUTEXUNLOCK(&(pLock->m_Lock));
		//Sleep�߳�
		FreeLongMinSleep();
	}

	//�������е������ʾ�Ѿ���ȡд��Ȩ��
MRSWLock_EnableWrite_Wait_Read_Clean:
	//�ȴ�������������
	while (MRSWLock_GetRead(pLock))
	{
		//�ȴ����еĶ��������?
		FreeLongMinSleep();
	}
}

void inline MRSWLock_DisableWrite(FreeLongMultiReadSingleWriteLock* pLock)
{
	MUTEXLOCK(&(pLock->m_Lock));
	{
		pLock->m_bWriteFlag=false;
	}
	MUTEXUNLOCK(&(pLock->m_Lock));			
}


int inline MRSWLock_AddRead(FreeLongMultiReadSingleWriteLock* pLock)
{
	while(1)                   //��ѭ���ȴ�,��ʹ�������̲߳�����Ҳ��ܿ����
	{
		MUTEXLOCK(&(pLock->m_Lock));
		{
			if(!pLock->m_bWriteFlag)
			{
				//���ۼӣ����ͷ�
				pLock->m_nReadCount++;
				//�����˳�
				MUTEXUNLOCK(&(pLock->m_Lock));
			
				return MRSWLock_GetRead(pLock);
			}
		}
		
		//���ͷ��ڲ����ٵȴ�
		MUTEXUNLOCK(&(pLock->m_Lock));
		//Sleep�߳�
		FreeLongMinSleep();
	}
}


int inline MRSWLock_DecRead(FreeLongMultiReadSingleWriteLock* pLock)
{
	int nRet=0;
	{
		MUTEXLOCK(&(pLock->m_Lock));
		{
			if(pLock->m_nReadCount>0)
			{
				//���ۼӣ����ͷ�
				pLock->m_nReadCount--;
				nRet=pLock->m_nReadCount;
			}
		}
		
		//���ͷ��ڲ����ٵȴ�
		MUTEXUNLOCK(&(pLock->m_Lock));
		return nRet;
	}
}


void inline MRSWLock_Read2Write(FreeLongMultiReadSingleWriteLock* pLock)
{
	while(1)                   //��ѭ���ȴ�
	{
		MUTEXLOCK(&(pLock->m_Lock));
		{
			if(!pLock->m_bWriteFlag)
			{
				//��������д��־Ϊ��
				pLock->m_bWriteFlag=true;

				if(pLock->m_nReadCount>0)
				{
					//���ۼӣ����ͷ�
					pLock->m_nReadCount--;
				}
				
				//�����˳�
				MUTEXUNLOCK(&(pLock->m_Lock));
				//����goto��ȷ��λ
				goto MRSWLock_Read2Write_Wait_Read_Clean;
			}
		}
		
		//���ͷ��ڲ����ٵȴ�
		MUTEXUNLOCK(&(pLock->m_Lock));
		//Sleep�߳�
		FreeLongMinSleep();
	}
	
	//�������е������ʾ�Ѿ���ȡд��Ȩ��
MRSWLock_Read2Write_Wait_Read_Clean:
	//�ȴ�������������
	while (MRSWLock_GetRead(pLock))
	{
		//�ȴ����еĶ��������?
		FreeLongMinSleep();
	}
}

#endif

