#ifndef MEMCACHEDCLIENT_H
#define	MEMCACHEDCLIENT_H

#include <cstdlib>
#include <iostream>
#include <string>
#include <libmemcached/memcached.h>

#define MAX_KEY_LEN             (256)
#define LELINKMC_MAXADDRLEN     (64)
#define MAX_DATA_LEN            (5120)

typedef struct __struMemCachedKey
{
    char Key[MAX_KEY_LEN];               
    int  keyLen; 
}MC_KEY,*PMC_KEY;
typedef struct __struMemCachedData
{
    int   dataLen;
    char  data[MAX_DATA_LEN];
    
}MC_DATA,*PMC_DATA;


//memcache-serverµÄÐÅÏ¢
typedef struct pps_mc_server_info_t
{
	char addr[LELINKMC_MAXADDRLEN];
	int  port;
}PPSMC_SERVERINFO;


using namespace std;

class MemCachedClient {
public:
    MemCachedClient();
    virtual ~MemCachedClient();
public:
    void InitMemCached(PPSMC_SERVERINFO* pServerList,int nCount);
    int Insert(const char* key, const char* value, time_t expiration = 0);
    int DeleteKey(const char* key);
    
    
    int MC_SET(const MC_KEY* key,MC_DATA* data,time_t expiration =0);
    int MC_GET(const MC_KEY* key,MC_DATA* data);
    int MC_UPDATE(const MC_KEY* key,MC_DATA* data);
    
private:
    memcached_st* memc; 
  

};

#endif	/* MEMCACHEDCLIENT_H */

