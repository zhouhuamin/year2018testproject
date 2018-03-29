/* 
 * File:   main.cpp
 * Author: root
 *
 * Created on 2016年11月29日, 下午3:32
 */

#include <cstdlib>
#include <iostream>
#include <string>
#include <libmemcached/memcached.h>
#include "CLelinkMemcachedHelper.h"

using namespace std;

struct T_SysEventData {
    char event_id[32];
    int is_data_full;
    char device_tag[32];
    int xml_data_len;
    char xml_data[];

};

CLelinkMemcachedHelper* m_pMemcachedHelper;

int PostEventDataToChannelControllerEx(const char *szKey, const char* szEventID, const char *szDeviceTag, int nDataLen, const char* szDataBuffer)
{
    {

        MC_KEY key;
        memset(&key, 0, sizeof (MC_KEY));

        MC_DATA mc_data;
        memset(&mc_data, 0, sizeof (MC_DATA));



        strcpy(key.Key, szKey);
        key.keyLen = strlen(key.Key);

        T_SysEventData* pReadData = (T_SysEventData*) mc_data.data;

        strcpy(pReadData->event_id, szEventID);
        strcpy(pReadData->device_tag, szDeviceTag);

        pReadData->is_data_full = 0; // 1;
        char szXMLGatherInfo[10 * 1024] = {0};

        if (nDataLen > 0 && szDataBuffer != NULL)
        {
            strncpy(szXMLGatherInfo, szDataBuffer, nDataLen);
            strcpy(pReadData->xml_data, szXMLGatherInfo);
            pReadData->xml_data_len = strlen(szXMLGatherInfo) + 1;


            mc_data.dataLen = sizeof (T_SysEventData) + pReadData->xml_data_len;

        } else
        {
            mc_data.dataLen = sizeof (T_SysEventData);
        }


        if (m_pMemcachedHelper->MC_SET(key, mc_data, 300))
        {
            return -1;
        }
    }

    return 0;
}


/*
 * 
 */
int main(int argc, char** argv)
{
    

    m_pMemcachedHelper = new CLelinkMemcachedHelper;

    PPSMC_SERVERINFO serverInfo = {0};

    strcpy(serverInfo.addr, "127.0.0.1");
    serverInfo.port = 11211;

    m_pMemcachedHelper->InitMemCached(&serverInfo, 1);    
    
    char szXMLGatherInfo1[10 * 1024] = {0};
    sprintf(szXMLGatherInfo1,
		"<IC>\n"
		"<DR_IC_NO>%s</DR_IC_NO>\n"
		"<IC_DR_CUSTOMS_NO></IC_DR_CUSTOMS_NO>\n"
		"<IC_CO_CUSTOMS_NO></IC_CO_CUSTOMS_NO>\n"
		"<IC_BILL_NO></IC_BILL_NO>\n"
		"<IC_GROSS_WT></IC_GROSS_WT>\n"
		"<IC_VE_CUSTOMS_NO></IC_VE_CUSTOMS_NO>\n"
		"<IC_VE_NAME></IC_VE_NAME>\n"
		"<IC_CONTA_ID></IC_CONTA_ID>\n"
		"<IC_ESEAL_ID></IC_ESEAL_ID>\n"
		"<IC_BUSS_TYPE></IC_BUSS_TYPE>\n"
		"<IC_EX_DATA></IC_EX_DATA>\n"
		"</IC>\n"
		, "SP00000000"
		);    
    
    PostEventDataToChannelControllerEx("1_IC_13_UP", "EG_FINISHED_IC", "IC", strlen(szXMLGatherInfo1), szXMLGatherInfo1);
    
    
    char szXMLGatherInfo2[10 * 1024] = {0};
	sprintf(szXMLGatherInfo2,
		"<WEIGHT>\n"
		"<GROSS_WT>%s</GROSS_WT>\n"
		"</WEIGHT>\n"
		, "67899"
		);    
    
    PostEventDataToChannelControllerEx("1_WEIGHT_11_UP", "EG_FINISHED_WEIGHT", "WEIGHT", strlen(szXMLGatherInfo2), szXMLGatherInfo2);
    
    
    
    char szXMLGatherInfo3[10 * 1024] = {0};
	sprintf(szXMLGatherInfo3,
		"<CAR>\n"
		"<VE_NAME></VE_NAME>\n"
		"<CAR_EC_NO>%s</CAR_EC_NO>\n"
		"<CAR_EC_NO2></CAR_EC_NO2>\n"
		"<VE_CUSTOMS_NO></VE_CUSTOMS_NO>\n"
		"<VE_WT></VE_WT>\n"
		"</CAR>\n"
		, "E4004F43405933A0"
		);    
    
    PostEventDataToChannelControllerEx("1_CAR_10_UP", "EG_FINISHED_CAR", "CAR", strlen(szXMLGatherInfo3), szXMLGatherInfo3);
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
//   //connect server
//
//   memcached_st *memc;
//   memcached_return rc;
//   memcached_server_st *server;
//   time_t expiration = 300;
//   uint32_t flags;
//
//   memc = memcached_create(NULL);
//   server = memcached_server_list_append(NULL,"localhost",11211,&rc);
//   rc=memcached_server_push(memc,server);
//   memcached_server_list_free(server);
//
//   string key = "key";
//   string value = "value";
//   size_t value_length = value.length();
//   size_t key_length = key.length();
//
//
//   //Save data
//
//   rc=memcached_set(memc,key.c_str(),key.length(),value.c_str(),value.length(),expiration,flags);
//   if(rc==MEMCACHED_SUCCESS)
//   {
//       cout<<"Save data:"<<value<<" sucessful!"<<endl;
//   }
//
//   //Get data
//
//   char* result = memcached_get(memc,key.c_str(),key_length,&value_length,&flags,&rc);
//   if(rc == MEMCACHED_SUCCESS)
//   {
//       cout<<"Get value:"<<result<<" sucessful!"<<endl;
//   }
//
//   //Delete data
//
//   rc=memcached_delete(memc,key.c_str(),key_length,expiration);
//   if(rc==MEMCACHED_SUCCESS)
//   {
//       cout<<"Delete key:"<<key<<" sucessful!"<<endl;
//   }
//
//   //free
//
//   memcached_free(memc);
   return 0;
}

