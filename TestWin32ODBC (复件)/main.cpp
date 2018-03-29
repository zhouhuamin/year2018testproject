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

/*
 * 
 */
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
    strcpy(szDBName, "gxhzdb");
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
    
    
    
    //printf("[???��?��???��?��??�?�?]\n");  
    dbcmd(dbprocess, "insert into customers(card_id, name, picture) values('789','王二', 'asd')");  
    if(dbsqlexec(dbprocess) == FAIL)  
    {  
        printf("insert into table 'test001' error.\n");  
        return -1;   
    }  
    printf("insert into table 'test001' success.\n");      
    
    
    
//    printf("[修改数据库表中的记录]\n");  
//    dbcmd(dbprocess, "update test001  set name='苏B12345' where id=4");  
//    if(dbsqlexec(dbprocess) == FAIL)  
//    {  
//        printf("update from table 'test001' error.\n");  
//        return -1;   
//    }  
//    printf("update from table 'test001' success.\n");      
    
    
    
      
    //关闭数据库连接  
    dbclose(dbprocess);  
    return 0;
}

