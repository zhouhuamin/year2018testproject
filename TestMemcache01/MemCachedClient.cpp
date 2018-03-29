#include "MemCachedClient.h"
#include <iostream>
#include <stdio.h>
using std::cout;
using std::endl;

MemCachedClient::MemCachedClient()
{
    memc = NULL;
  
}

MemCachedClient::~MemCachedClient()
{
    if(memc != NULL)
        memcached_free(memc);

}

void MemCachedClient::InitMemCached(PPSMC_SERVERINFO* pServerList, int nCount)
{
    if(memc != NULL)
    {
        return;
    }

    memcached_return rc;
    memcached_server_st *server = NULL;

    memc = memcached_create(NULL);

    for (int i = 0; i < nCount; i++)
    {
        server = memcached_server_list_append(server, pServerList[i].addr, pServerList[i].port, &rc);
    }

    rc = memcached_server_push(memc, server);

    if(MEMCACHED_SUCCESS != rc)
    {
        cout << "memcached_server_push failed! rc: " << rc << endl;
    }

    memcached_server_list_free(server);


    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_DISTRIBUTION, MEMCACHED_DISTRIBUTION_CONSISTENT);
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_RETRY_TIMEOUT, 5);
    //  memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_REMOVE_FAILED_SERVERS, 1) ;  // 同时设置MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT 和 MEMCACHED_BEHAVIOR_AUTO_EJECT_HOSTS  
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_SERVER_FAILURE_LIMIT, 5);
    memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_AUTO_EJECT_HOSTS, true);

}

int MemCachedClient::Insert(const char* key, const char* value, time_t expiration)
{
    if(NULL == key || NULL == value)
    {
        return -1;
    }

    uint32_t flags = 0;

    memcached_return rc;

    rc = memcached_set(memc, key, strlen(key), value, strlen(value), expiration, flags);

    // insert ok
    if(MEMCACHED_SUCCESS == rc)
    {
        
        return 0;
    }
    else
    {
        
        return -1;
    }
};



int MemCachedClient::MC_SET(const MC_KEY* key, MC_DATA* data, time_t expiration)
{
    if(NULL == memc)
        return -1;



    if(NULL == key || key->keyLen < 1)
        return -1;

    if(NULL == data)
        return -1;

    uint32_t flags = 0;

    memcached_return rc;

    rc = memcached_set(memc, key->Key, key->keyLen, (const char*) data->data, data->dataLen, expiration, flags);

    // insert ok
    if(MEMCACHED_SUCCESS != rc)
    {
        
        return -1;
    }
    else
    {
        return 0;
    }
}

int MemCachedClient::MC_GET(const MC_KEY* key, MC_DATA* data)
{
    if(NULL == memc)
        return -1;

    if(NULL == key || key->keyLen < 1)
        return -1;

    if(NULL == data || NULL == data->data)
        return -1;

    uint32_t flags = 0;

    memcached_return rc;

    size_t value_length;
    char* value = memcached_get(memc, key->Key, key->keyLen, &value_length, &flags, &rc);

    // get ok
    if(rc == MEMCACHED_SUCCESS && value!=NULL )
    {
        memset(data->data, 0, MAX_DATA_LEN);
        data->dataLen = value_length;
        memcpy(data->data, value, value_length);

        free(value);
        return 0;
    }
  
    return rc;
}

int MemCachedClient::MC_UPDATE(const MC_KEY* key, MC_DATA* data)
{
    return MC_SET(key, data);
}

int MemCachedClient::DeleteKey(const char* key)
{
    memcached_delete(memc, key, strlen(key), 0);
   

    return 0;
    
}