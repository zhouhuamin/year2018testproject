/* 
 * File:   main.c
 * Author: root
 *
 * Created on 2017年8月1日, 下午2:42
 */

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

/*
 * 
 */

void createdb() {
    sqlite3 *db = NULL;
    char *zErrMsg = 0;
    int rc;

    //打开指定的数据库文件,如果不存在将创建一个同名的数据库文件  
    rc = sqlite3_open("jzfrontdb.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s/n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    } else
        printf("You have opened a sqlite3 database named jzfrontdb.db successfully!/nCongratulations! Have fun !  ^-^ /n");

    sqlite3_close(db); //关闭数据库        
}

void insertdb() {
    sqlite3 *db = NULL;
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open("jzfrontdb.db", &db); //打开指定的数据库文件,如果不存在将创建一个同名的数据库文件
    if (rc) {
        fprintf(stderr, "Can't open database: %s/n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    } else
        printf("You have opened a sqlite3 database named jzfrontdb.db successfully!/nCongratulations! Have fun !  ^-^ /n");

    //创建一个表,如果该表存在，则不创建，并给出提示信息，存储在 zErrMsg 中
    //    char *sql = " CREATE TABLE SensorData(\
//      ID INTEGER PRIMARY KEY,\
//     SensorID INTEGER,\
//      SiteNum INTEGER,\
//      Time VARCHAR(12),\
//      SensorParameter REAL\
//      );";
    //    sqlite3_exec(db, sql, 0, 0, &zErrMsg);

    char *sql = "";

    //插入数据 
    sql = "INSERT INTO T_GATHERINFO(AREA_ID, CHNL_NO, I_E_TYPE, SEQ_NO, VE_NAME,GATHER_TIME,IS_REGATHER) \
        VALUES('HTM0000000','TT10000001','I','20170511010158712034','闽DD0999','2017-08-03 16:07:48', '0');";
    sqlite3_exec(db, sql, 0, 0, &zErrMsg);

    sqlite3_close(db); //关闭数据库
    return;
}

int selectdb() {
    sqlite3 *db = NULL;
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open("jzfrontdb.db", &db); //打开指定的数据库文件,如果不存在将创建一个同名的数据库文件  
    if (rc) {
        fprintf(stderr, "Can't open database: %s/n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }
    else printf("You have opened a sqlite3 database named jzfrontdb.db successfully!/nCongratulations! Have fun !  ^-^ /n");

    //创建一个表,如果该表存在，则不创建，并给出提示信息，存储在 zErrMsg 中  
    char *sql = "";

    int nrow = 0, ncolumn = 0;
    char **azResult; //二维数组存放结果  
    //查询数据  
    /* 
    int sqlite3_get_table(sqlite3*, const char *sql,char***result , int *nrow , int *ncolumn ,char **errmsg ); 
    result中是以数组的形式存放你所查询的数据，首先是表名，再是数据。 
    nrow ,ncolumn分别为查询语句返回的结果集的行数，列数，没有查到结果时返回0 
     */
    sql = "SELECT * FROM T_GATHERINFO ";
    sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);
    int i = 0;
    printf("row:%d column=%d \n", nrow, ncolumn);
    printf("\nThe result of querying is : \n");

    for (i = 0; i < (nrow + 1) * ncolumn; i++)
        printf("azResult[%d] = %s\n", i, azResult[i]);
    //释放掉  azResult 的内存空间  
    sqlite3_free_table(azResult);

#ifdef _DEBUG_  
    printf("zErrMsg = %s /n", zErrMsg);
#endif  
    sqlite3_close(db); //关闭数据库  
    return 0;
}

static int callback(void *data, int argc, char **argv, char **azColName){
   int i;
   fprintf(stderr, "%s: ", (const char*)data);
   for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

int selectdb2()
{
   sqlite3 *db;
   char *zErrMsg = 0;
   int rc;
   char *sql;
   const char* data = "Callback function called";

   /* Open database */
   rc = sqlite3_open("jzfrontdb.db", &db);
   if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      exit(0);
   }else{
      fprintf(stderr, "Opened database successfully\n");
   }

   /* Create SQL statement */
   sql = "SELECT * from T_GATHERINFO";

   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
   if( rc != SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }else{
      fprintf(stdout, "Operation done successfully\n");
   }
   sqlite3_close(db);
   return 0;
}


void updatedb() 
{
    sqlite3 *db = NULL;
    char *zErrMsg = 0;
    int rc;

    rc = sqlite3_open("jzfrontdb.db", &db); //打开指定的数据库文件,如果不存在将创建一个同名的数据库文件
    if (rc) {
        fprintf(stderr, "Can't open database: %s/n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    } else
    printf("You have opened a sqlite3 database named jzfrontdb.db successfully!/nCongratulations! Have fun !  ^-^ /n");

    char *sql = "";

    //插入数据 
    sql = "update T_GATHERINFO set CHECK_RESULT_PASS='00000000001001000000',VE_NAME_PASS='闽DA9153',\
            OP_REASON_PASS='车号不一致',PASS_TIME='2017-05-10 19:38:19' where SEQ_NO='20170511010158712034';";
    sqlite3_exec(db, sql, 0, 0, &zErrMsg);

    sqlite3_close(db); //关闭数据库
    return;
}



int main(int argc, char** argv) {

    //selectdb2();
    
    insertdb();
    //updatedb();


    return (EXIT_SUCCESS);
}

