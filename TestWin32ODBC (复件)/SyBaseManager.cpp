/*    
* SyBaseManager.cpp    
*    
*    Created .: Feb 18, 2009    
*            Author: Steven Wee    
*/    
#include "SyBaseManager.h"

SybaseManager::SybaseManager(std::string hosts, std::string userName, std::string password, std::string dbName, unsigned int port)    
{    
        IsConnected = false;    
        this->setHosts(hosts);    
        this->setUserName(userName);    
        this->setPassword(password);    
        this->setDBName(dbName);    
        this->setPort(port);    
}    

SybaseManager::~SybaseManager()    
{    
        destroyConnection();    
}    

void SybaseManager::setDBName(string dbName)    
{    
    DBNAME = dbName;
}    

void SybaseManager::setHosts(string hosts)    
{    
    HOSTS = hosts;
}    

void SybaseManager::setPassword(string password)    
{    
    PASSWORD = password;
}    

void SybaseManager::setPort(unsigned int port)    
{    
    DEFAULTPORT = port;
}    

void SybaseManager::setUserName(string userName)    
{    
    USERNAME = userName;
}    

void SybaseManager::initConnection()    
{    
        string Charset = "UTF-8";    
        dbinit();    
        LOGINREC *loginREC = dblogin();    
        DBSETLUSER(loginREC, this->USERNAME.c_str());    
        DBSETLPWD(loginREC, this->PASSWORD.c_str());    
        DBSETLCHARSET(loginREC, Charset.c_str());    
        dbProcess = dbopen(loginREC, this->HOSTS.c_str());    
        if ( dbProcess == FAIL )    
        {    
                std::cout << "Connect to SQL Server failed!" << std::endl; 
                return;
        }    
        if ( dbuse( dbProcess, this->DBNAME.c_str() ) == FAIL )    
        {    
                std::cout << "Use table failed!" << std::endl;    
        }    
}    

bool SybaseManager::runSQLCommand( string sql )    
{    
        dbcmd(dbProcess, sql.c_str());    
        if ( dbsqlexec(dbProcess) == FAIL )    
        {    
                std::cout << "Query from database failed!" << std::endl;    
        }    
        DBINT result_code;    
        vector<string> objectValue;    
//        StringTools stringTools;    
//
//        sql = stringTools.filterString(sql);    

        while ( (result_code = dbresults(dbProcess)) != NO_MORE_RESULTS )    
        {    
                struct Column    
                {    
                        char* colName;    
                        char* colBuffer;    
                        int colType, colSize, colStatus;    
                } *columns, *pCol;    
                int nColumns;    
                int rowNo;    
                if ( result_code == SUCCEED )    
                {    
                        nColumns = dbnumcols(dbProcess);    
                        if ( (columns = (Column*)calloc(nColumns, sizeof(struct Column))) == NULL )    
                        {    
                                std::cout << "Error at bind data" << std::endl;    
                                return false;    
                        }    
                        for ( pCol = columns; pCol - columns < nColumns; pCol++ )    
                        {    
                                int colNo = pCol - columns + 1;    
                                pCol ->colName = dbcolname(dbProcess, colNo);    
                                pCol ->colType = dbcoltype(dbProcess, colNo);    
                                pCol ->colSize = dbcollen(dbProcess, colNo);    
                                if ( SYBCHAR != pCol ->colType )    
                                {    
                                        pCol ->colSize = dbwillconvert(pCol ->colType, SYBCHAR);    
                                }    

                                if ( (pCol ->colBuffer = (char*)calloc(1, pCol ->colSize + 1)) == NULL )    
                                {    
                                        std::cout << "Check column buffer error!" << std::endl;    
                                        return false;    
                                }    

                                if ( dbbind(dbProcess, colNo, STRINGBIND, pCol ->colSize + 1, (BYTE*)pCol ->colBuffer) == FAIL )    
                                {    
                                        std::cout << "Running dbbind() error!" << std::endl;    
                                        return false;    
                                }    

                                if ( dbnullbind(dbProcess, colNo, &pCol ->colStatus) == FAIL )    
                                {    
                                        std::cout << "Running dbnullbind() error!" << std::endl;    
                                        return false;    
                                }    
                        }    

                        while ( (rowNo = dbnextrow(dbProcess)) != NO_MORE_ROWS )    
                        {    
                                objectValue.clear();    
                                switch ( rowNo )    
                                {    
                                case REG_ROW:    
                                        for ( pCol = columns; pCol - columns < nColumns; pCol++ )    
                                        {    
                                                const char* columnBuffer = pCol ->colStatus == -1 ? "NULL" : pCol ->colBuffer;    
                                                //objectValue.push_back(stringTools.Trim(columnBuffer));        //        std::cout << columnBuffer << std::endl;    
                                        }    
                                        break;    
                                case BUF_FULL:    
                                        assert( rowNo != BUF_FULL );    
                                        break;    
                                case FAIL:    
                                        std::cout << "Get result error!" << std::endl;    
                                        break;    
                                default:    
                                        std::cout << "Get result ignore, row number:" << rowNo << std::endl;    
                                }    
                                this->resultList.push_back(objectValue);    
                        }    
                        for ( pCol = columns; pCol - columns < nColumns; pCol++ )    
                        {    
                                free( pCol ->colBuffer );    
                        }    
                        free( columns );    
                        /*    
                        if ( DBCOUNT(dbProcess) > -1 )    
                        {    
                                std::cout << "Affected rows:" << DBCOUNT(dbProcess) << std::endl;    
                        }    
                        */    
                        if ( dbhasretstat(dbProcess) == TRUE )    
                        {    
                                std::cout << "Procedure returned " << dbhasretstat(dbProcess) << std::endl;    
                        }    
                }    
        }    
        return true;    
}    

void SybaseManager::destroyConnection()    
{    
        dbclose(dbProcess);    
}    

bool SybaseManager::getConnectionStatus()    
{    
        return IsConnected;    
}    

vector< vector<string> > SybaseManager::getResult()    
{    
        return this->resultList;    
} 