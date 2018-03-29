/* 
 * File:   main.cpp
 * Author: root
 *
 * Created on 2015年11月9日, 下午3:29
 */

#include <string.h>
#include <cstdlib>
#include <stdio.h>
#include <string>
#include "IDDetect.h"
#include <boost/timer.hpp>

using namespace std;
using namespace boost;

/*
 * 
 */
int main(int argc, char** argv)
{
    ContainerID code_id;
    memset(&code_id, 0, sizeof (ContainerID));
    
    char szRecoFile[256] = {0};
    char req_sequence[256] = {0};
    int ret = 0;
    
    
    timer tmptime;
    string strSeq       = "";
    string strFileName  = "/root/testpic/";
    strSeq = "26efef10-fe76-4c53-81d0-49a8b38a2a3c";
    strFileName         += "26efef10-fe76-4c53-81d0-49a8b38a2a3c.jpg";
    //printf("......reco %s\n",strFileName.c_str());
    ret = PathReadCode((char*)strFileName.c_str(), &code_id); //调用函数 该函数在IDDetect.cpp中
    printf("%s,%s\t\t\t\t %s,%.3lf\n", strSeq.c_str(), code_id.ID, code_id.Type, tmptime.elapsed());
    
    
    tmptime.restart();
    strFileName  = "/root/testpic/";
    strSeq = "2ec11e8e-2e94-4bce-aae6-69a69e6caebc";
    strFileName         += "2ec11e8e-2e94-4bce-aae6-69a69e6caebc.jpg";
    ret = PathReadCode((char*)strFileName.c_str(), &code_id); //调用函数 该函数在IDDetect.cpp中
    printf("%s,%s\t\t %s,%.3lf\n", strSeq.c_str(), code_id.ID, code_id.Type, tmptime.elapsed());
    
    
    tmptime.restart();
    strFileName  = "/root/testpic/";
    strSeq = "658aac59-ef9b-4235-9e94-cdfe705ab300";
    strFileName         += "658aac59-ef9b-4235-9e94-cdfe705ab300.jpg";
    ret = PathReadCode((char*)strFileName.c_str(), &code_id); //调用函数 该函数在IDDetect.cpp中
    printf("%s,%s\t\t\t\t %s,%.3lf\n", strSeq.c_str(), code_id.ID, code_id.Type, tmptime.elapsed());
    
    
    tmptime.restart();
    strFileName  = "/root/testpic/";
    strSeq = "8ca108a1-ce9a-4f72-b817-b37aa4d9127c";
    strFileName         += "8ca108a1-ce9a-4f72-b817-b37aa4d9127c.jpg";
    ret = PathReadCode((char*)strFileName.c_str(), &code_id); //调用函数 该函数在IDDetect.cpp中
    printf("%s,%s\t\t\t\t %s,%.3lf\n", strSeq.c_str(), code_id.ID, code_id.Type, tmptime.elapsed());
    
    
    tmptime.restart();
    strFileName  = "/root/testpic/";
    strSeq = "ce94f079-b0ec-4868-82f9-cea251ee87d3";
    strFileName         += "ce94f079-b0ec-4868-82f9-cea251ee87d3.jpg";
    ret = PathReadCode((char*)strFileName.c_str(), &code_id); //调用函数 该函数在IDDetect.cpp中
    printf("%s,%s\t\t\t\t %s,%.3lf\n", strSeq.c_str(), code_id.ID, code_id.Type, tmptime.elapsed());
    
    
    tmptime.restart();
    strFileName  = "/root/testpic/";
    strSeq = "e9c34054-4151-440d-9086-57d671a57b30";
    strFileName         += "e9c34054-4151-440d-9086-57d671a57b30.jpg";
    ret = PathReadCode((char*)strFileName.c_str(), &code_id); //调用函数 该函数在IDDetect.cpp中
    printf("%s,%s\t\t %s,%.3lf\n", strSeq.c_str(), code_id.ID, code_id.Type, tmptime.elapsed());
    return 0;
}

