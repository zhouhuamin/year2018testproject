/* 
 * File:   CLelinkMemcachedHelper.cpp
 * Author: root
 * 
 * Created on 2014年1月9日, 下午9:54
 */

#include "CLelinkMemcachedHelper.h"
#include <stdio.h>

CLelinkMemcachedHelper::CLelinkMemcachedHelper()
{

    m_nAccessCount = 0;
    for (int i = 0; i < MAX_MEMCACHED_CONNECTIONS; i++)
    {
        MemCachedClient* pMemCachedClient = new MemCachedClient;
        m_pMemCachedClient[i] = pMemCachedClient;
    }

}

CLelinkMemcachedHelper::~CLelinkMemcachedHelper()
{
    for (int i = 0; i < MAX_MEMCACHED_CONNECTIONS; i++)
    {
        MemCachedClient* pMemCachedClient = m_pMemCachedClient[i];
        delete pMemCachedClient;
    }

}

int CLelinkMemcachedHelper::InitMemCached(PPSMC_SERVERINFO* pServerList, int nCount)
{

    for (int i = 0; i < MAX_MEMCACHED_CONNECTIONS; i++)
    {
        MemCachedClient* pMemCachedClient = m_pMemCachedClient[i];
        pMemCachedClient->InitMemCached(pServerList, nCount);
    }
}

MemCachedClient* CLelinkMemcachedHelper::GetMemCachedClient()
{
    CMutexGuard guard(access_lock_);
    MemCachedClient* pClient = m_pMemCachedClient[m_nAccessCount];

    m_nAccessCount++;
    if(m_nAccessCount > MAX_MEMCACHED_CONNECTIONS - 1)
    {
        m_nAccessCount = 0;
    }

    return pClient;
}



int CLelinkMemcachedHelper::MC_GET(MC_KEY& key, MC_DATA& data)
{
    MemCachedClient* pMemcacheClient = GetMemCachedClient();
    return pMemcacheClient->MC_GET(&key, &data);
}

int CLelinkMemcachedHelper::MC_SET(MC_KEY& key, MC_DATA& data,time_t expiration)
{
    MemCachedClient* pMemcacheClient = GetMemCachedClient();
    return pMemcacheClient->MC_SET(&key, &data,expiration);
}

int CLelinkMemcachedHelper::DeleteKey(const char* key)
{
    MemCachedClient* pMemcacheClient = GetMemCachedClient();
    return pMemcacheClient->DeleteKey(key);
}



