/* 
 * File:   CLelinkMemcachedHelper.h
 * Author: root
 *
 * Created on 2014年1月9日, 下午9:54
 */

#ifndef CLELINKMEMCACHEDHELPER_H
#define	CLELINKMEMCACHEDHELPER_H

#include "MemCachedClient.h"
#include "MutexGuard.h"

#define MAX_MEMCACHED_CONNECTIONS      (0X02) 

class CLelinkMemcachedHelper {
public:
    CLelinkMemcachedHelper();
    virtual ~CLelinkMemcachedHelper();
    
public:
    int InitMemCached(PPSMC_SERVERINFO* pServerList, int nCount);
  
    int MC_GET(MC_KEY& key,MC_DATA& data);
    int MC_SET(MC_KEY& key,MC_DATA& data,time_t expiration=259200);
    int DeleteKey(const char* key);
    
private:
    MemCachedClient* GetMemCachedClient();
private:
    
    MemCachedClient* m_pMemCachedClient[MAX_MEMCACHED_CONNECTIONS];
    CSysMutex        access_lock_;
    int              m_nAccessCount;

};

#endif	/* CLELINKMEMCACHEDHELPER_H */

