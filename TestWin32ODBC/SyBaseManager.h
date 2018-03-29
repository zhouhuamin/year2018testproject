/*    
* SyBaseManager.h    
*    
*    Created .: Feb 18, 2009    
*            Author: Steven Wee    
*/    

#ifndef SYBASEMANAGER_H_    
#define SYBASEMANAGER_H_    

//#include "../Common/CheckStringTools.h"    

#include <string>    
#include <vector>    
#include <iostream>    
#include <assert.h>    
#include <errno.h>    
#include <stdlib.h>    
#include <string.h>    

#include <sybfront.h>    
#include <sybdb.h>    

using namespace std;    

class SybaseManager    
{    
public:    
        SybaseManager(std::string hosts, std::string userName, std::string password, std::string dbName, unsigned int port);    
        ~SybaseManager();    
        /*    
         * Init SQL Server    
         * @param hosts:         Host IP address    
         * @param userName:        Login UserName    
         * @param password:        Login Password    
         * @param dbName:        Database Name    
         * @param port:                Host listen port number    
         */    
        void initConnection();    
        /*    
         * Making query from database    
         * @param mysql:        MySQL Object    
         * @param sql:                Running SQL command    
         */    
        bool runSQLCommand(std::string sql);    
        /**    
         * Destroy MySQL object    
         * @param mysql                MySQL object    
         */    
        void destroyConnection();    
        bool getConnectionStatus();    
        vector<vector<string> > getResult();    
protected:    
        void setUserName(std::string userName);    
        void setHosts(std::string hosts);    
        void setPassword(std::string password);    
        void setDBName(std::string dbName);    
        void setPort(unsigned int port);    
private:    
        bool IsConnected;    
        DBPROCESS *dbProcess;    
        vector< vector<string> > resultList;    
        unsigned int DEFAULTPORT;    
        std::string HOSTS;    
        std::string USERNAME;    
        std::string PASSWORD;    
        std::string DBNAME;    
};    

#endif /* SYBASEMANAGER_H_ */ 
