/* 
 * File:   main.cpp
 * Author: root
 *
 * Created on 2017�?6??21??, �???9:20
 */

#include <cstdlib>
#include <stdio.h>  
#include <string.h>  
#include <stdlib.h>  
#include <unistd.h> 


#include "SyBaseManager.h"

using namespace std;


int main(int argc, char** argv)
{
//    std::string hosts       = "192.168.1.13:56299";
//    std::string userName    = "sa";
//    std::string password    = "sa";
//    std::string dbName      = "jztest1";
//    unsigned int port       = 56299;
//    SybaseManager manager(hosts, userName, password, dbName, port);
//    manager.initConnection();
//    
//    std::string sql = "select id,name from test001";
//    manager.runSQLCommand(sql);
//    manager.destroyConnection();
    
    
    
    string Charset = "UTF-8";  
  char szUsername[32] = {0};  
    char szPassword[32] = {0};  
    char szDBName[32] = {0};   //?��??�???  
    char szServer[32] = {0};  //?��??�????��??:�???  
    
    strcpy(szUsername, "sa");
    strcpy(szPassword, "sa");
    strcpy(szDBName, "KJAutoData");
    strcpy(szServer, "192.168.1.16:1433");
   
    //??�???db-library  
    dbinit();  
         
    //�??��?��??�?  
    LOGINREC *loginrec = dblogin();  
    DBSETLUSER(loginrec, szUsername);         
    DBSETLPWD(loginrec, szPassword);  
    DBSETLCHARSET(loginrec, Charset.c_str());  
    DBPROCESS *dbprocess = dbopen(loginrec, szServer);//�??��?��??�?  
    if(dbprocess == FAIL)  
    {  
        printf("Conect to MS SQL SERVER fail, exit!\n");  
        return -1;   
    }  
    printf("Connect to MS SQL SERVER success!\n");  
         
    if(dbuse(dbprocess, szDBName) == FAIL)  
        printf("Open database failed!\n");  
    else  
        printf("Open database success!\n");  
         
//    //?��?��?��??�?  
//    //printf("[?��?��?��??�?�?]\n");  
//    dbcmd(dbprocess, "select id,card_id,name from customers");  
//    if(dbsqlexec(dbprocess) == FAIL)  
//    {  
//        printf("Query table 'customers' error.\n");  
//        return -1;   
//    }  
//       
//    DBINT result_code;  
//    char szID[100]={0};  
//    char szCardID[80]={0};  
//    char szName[10]={0}; 
//    char szUserName[100]={0}; 
//    int rows = 0;  
//    while ((result_code = dbresults(dbprocess)) != NO_MORE_RESULTS){  
//        if (result_code == SUCCEED){  
//            //dbbind(dbprocess, 1, CHARBIND, (DBINT)0, (BYTE*)szStuID);  
//            //dbbind(dbprocess, 2, CHARBIND, (DBCHAR)0, (BYTE*)szName);  
//            //dbbind(dbprocess, 3, CHARBIND, (DBCHAR)0, (BYTE*)szAge);  
//            //dbbind(dbprocess, 4, CHARBIND, (DBCHAR)0, (BYTE*)szUserName);
//            dbbind(dbprocess, 1, CHARBIND, (DBINT)0, (BYTE*)szID);  
//            dbbind(dbprocess, 2, CHARBIND, (DBCHAR)0, (BYTE*)szCardID);  
//            dbbind(dbprocess, 3, CHARBIND, (DBCHAR)0, (BYTE*)szName);
//            printf("ID\tCARDID\tNAME\n");  
//            while (dbnextrow(dbprocess) != NO_MORE_ROWS){                          
//                printf("%s\t", szID);  
//                printf("%s\t", szCardID);  
//                printf("%s\n", szName); 
//               // printf("%s\n", szUserName); 
//            }  
//        }  
//    }         
   
//    //printf("[???��?��???��?��??�?�?]\n");  
//    dbcmd(dbprocess, "insert into test001(id, name) values(5,'苏A12345')");  
//    if(dbsqlexec(dbprocess) == FAIL)  
//    {  
//        printf("insert into table 'test001' error.\n");  
//        return -1;   
//    }  
//    printf("insert into table 'test001' success.\n");  
      
//    printf("[删除数据库表中的记录]\n");  
//    dbcmd(dbprocess, "delete from test001 where id=5");  
//    if(dbsqlexec(dbprocess) == FAIL)  
//    {  
//        printf("delete from table 'test001' error.\n");  
//        return -1;   
//    }  
//    printf("delete from table 'test001' success.\n");  
    
    
    
//    //printf("[???��?��???��?��??�?�?]\n");  
//    dbcmd(dbprocess, "insert into [卡号照片]([编号], [身份证号码], [身份证照片], [发卡照片], [收卡照片]) values('','999', '','', '')");  
//    if(dbsqlexec(dbprocess) == FAIL)  
//    {  
//        printf("insert into table '[卡号照片]' error.\n");  
//        return -1;   
//    }  
//    printf("insert into table '[卡号照片]' success.\n");      
  
    
    
    
    
    
    
    
//    long result, isiz;
//    FILE *fp;
//    char *blob;
//    if ((fp = fopen("/root/recvpic/20170509153528176_big.jpg", "rb")) == NULL) 
//    {
//            fprintf(stderr, "Cannot open input file: %s\n", argv[1]);
//            return 2;
//    }
//    result = fseek(fp, 0, SEEK_END);
//    isiz = ftell(fp);
//    result = fseek(fp, 0, SEEK_SET);
//
//    blob = (char *) malloc(isiz);
//    fread((void *) blob, isiz, 1, fp);
//    fclose(fp);       
//    
//    //?��?��?��??�?  
//    //printf("[?��?��?��??�?�?]\n");  
//    dbcmd(dbprocess, "select [身份证照片] from [卡号照片] where [编号]='789'");  
//    if(dbsqlexec(dbprocess) == FAIL)  
//    {  
//        printf("Query table '[卡号照片]' error.\n");  
//        return -1;   
//    }  
//    
//    char objname[256];
//    unsigned char *textPtr, *timeStamp;
//    DBINT result_code;  
//    char szID[100]={0};  
//    char szCardID[80]={0};  
//    char szName[10]={0}; 
//    char szUserName[100]={0}; 
//    int rows = 0;  
//    if ((result_code = dbresults(dbprocess)) != NO_MORE_RESULTS)
//    {  
//        if (result_code == SUCCEED)
//        {  
//            if ((result = dbnextrow(dbprocess)) != NO_MORE_ROWS) 
//            {
//                result = REG_ROW;
//                result = DBTXPLEN;
//                strcpy(objname, "[卡号照片].[身份证照片]");
//                textPtr = dbtxptr(dbprocess, 1);
//                timeStamp = dbtxtimestamp(dbprocess, 1);
//
//                if (dbwritetext(dbprocess, objname, textPtr, DBTXPLEN, timeStamp, TRUE, isiz, (BYTE*) blob) != SUCCEED)
//                {
//                    printf("save pic to table 'customers' error.\n");  
//                    return -1;  
//                }
//                else
//                {
//                    printf("save pic to table 'customers' succ.\n");  
//                }
//            }
//        }  
//    }      
    
       
    
//    printf("[修改数据库表中的记录]\n");  
//    dbcmd(dbprocess, "update [卡号照片]  set [编号]='12345' where [身份证号码]='999' and [编号]=''");  
//    if(dbsqlexec(dbprocess) == FAIL)  
//    {  
//        printf("update from table '[卡号照片]' error.\n");  
//        return -1;   
//    }  
//    printf("update from table '[卡号照片]' success.\n");      
    
    
    //?��?��?��??�?  
    //printf("[?��?��?��??�?�?]\n");  
    dbcmd(dbprocess, "select [毛重时间],[皮重时间] from [称重信息]");  
    if(dbsqlexec(dbprocess) == FAIL)  
    {  
        printf("Query table '称重信息' error.\n");  
        return -1;   
    }  
       
    DBINT result_code;  
    char szID[100]={0};  
    char szCardID[80]={0};  
    
    
    char datestring[256] = {0};
    DBDATEREC dateinfo;
    DBDATETIME mydatetime;
    
    char datestring2[256] = {0};
    DBDATEREC dateinfo2;
    DBDATETIME mydatetime2;    
    
    

    int rows = 0;  
    while ((result_code = dbresults(dbprocess)) != NO_MORE_RESULTS)
    {  
        if (result_code == SUCCEED)
        {  
            dbbind(dbprocess, 1, CHARBIND, (DBINT)0, (BYTE*)szID);  
            dbbind(dbprocess, 2, CHARBIND, (DBCHAR)0, (BYTE*)szCardID);  

            printf("ID\t毛重时间\t皮重时间\n");  
            while (dbnextrow(dbprocess) != NO_MORE_ROWS)
            {                          
                printf("%s\t", szID);  
                printf("%s\n", szCardID);  
                
		dbconvert(dbprocess, dbcoltype(dbprocess, 1), dbdata(dbprocess, 1), dbdatlen(dbprocess, 1), SYBCHAR, (BYTE*) datestring, -1);
                
                dbconvert(dbprocess, dbcoltype(dbprocess, 2), dbdata(dbprocess, 2), dbdatlen(dbprocess, 2), SYBCHAR, (BYTE*) datestring2, -1);

		printf("%s\n", datestring);    
                printf("%s\n", datestring2);    
                
		memcpy(&mydatetime, (DBDATETIME *) (dbdata(dbprocess, 1)), sizeof(DBDATETIME));
		dbdatecrack(dbprocess, &dateinfo, &mydatetime);  
                
                memcpy(&mydatetime2, (DBDATETIME *) (dbdata(dbprocess, 2)), sizeof(DBDATETIME));
		dbdatecrack(dbprocess, &dateinfo2, &mydatetime2);                    
                
#ifdef MSDBLIB
		printf("\tYear = %d.\n", dateinfo.year);
		printf("\tMonth = %d.\n", dateinfo.month);
		printf("\tDay of month = %d.\n", dateinfo.day);
		printf("\tDay of year = %d.\n", dateinfo.dayofyear);
		printf("\tDay of week = %d.\n", dateinfo.weekday);
		printf("\tHour = %d.\n", dateinfo.hour);
		printf("\tMinute = %d.\n", dateinfo.minute);
		printf("\tSecond = %d.\n", dateinfo.second);
		printf("\tMillisecond = %d.\n", dateinfo.millisecond);
#else
		printf("\tYear = %d.\n", dateinfo.dateyear);
		printf("\tMonth = %d.\n", dateinfo.datemonth);
		printf("\tDay of month = %d.\n", dateinfo.datedmonth);
		printf("\tDay of year = %d.\n", dateinfo.datedyear);
		printf("\tDay of week = %d.\n", dateinfo.datedweek);
		printf("\tHour = %d.\n", dateinfo.datehour);
		printf("\tMinute = %d.\n", dateinfo.dateminute);
		printf("\tSecond = %d.\n", dateinfo.datesecond);
		printf("\tMillisecond = %d.\n", dateinfo.datemsecond);
#endif        
                
                
#ifdef MSDBLIB
		printf("\tYear = %d.\n", dateinfo2.year);
		printf("\tMonth = %d.\n", dateinfo2.month);
		printf("\tDay of month = %d.\n", dateinfo2.day);
		printf("\tDay of year = %d.\n", dateinfo2.dayofyear);
		printf("\tDay of week = %d.\n", dateinfo2.weekday);
		printf("\tHour = %d.\n", dateinfo2.hour);
		printf("\tMinute = %d.\n", dateinfo2.minute);
		printf("\tSecond = %d.\n", dateinfo2.second);
		printf("\tMillisecond = %d.\n", dateinfo2.millisecond);
#else
		printf("\tYear = %d.\n", dateinfo2.dateyear);
		printf("\tMonth = %d.\n", dateinfo2.datemonth);
		printf("\tDay of month = %d.\n", dateinfo2.datedmonth);
		printf("\tDay of year = %d.\n", dateinfo2.datedyear);
		printf("\tDay of week = %d.\n", dateinfo2.datedweek);
		printf("\tHour = %d.\n", dateinfo2.datehour);
		printf("\tMinute = %d.\n", dateinfo2.dateminute);
		printf("\tSecond = %d.\n", dateinfo2.datesecond);
		printf("\tMillisecond = %d.\n", dateinfo2.datemsecond);
#endif                 
                
                
                
            }  
        }  
    }      
    

    //关闭数据库连接  
    dbclose(dbprocess);  
    return 0;
}



