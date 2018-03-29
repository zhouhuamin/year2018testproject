/* 
 * File:   main.cpp
 * Author: root
 *
 * Created on 2017年1月17日, 下午2:19
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
 
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include <string>

#include "httpclient.h"

using namespace std;


 
FILE *fp;  //定义FILE类型指针
//这个函数是为了符合CURLOPT_WRITEFUNCTION而构造的
//完成数据保存功能
size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)  
{
    int written = fwrite(ptr, size, nmemb, (FILE *)fp);
    return written;
}
 

/*
 * 
 */
int main(int argc, char** argv)
{
//    CURL *curl;
////    if (argc != 3)
////    {
////		fprintf(stderr, "usage: %s url filename\n", argv[0]);
////		exit(-1);
////    }
//    curl_global_init(CURL_GLOBAL_ALL);  
//    curl = curl_easy_init();
//    
//    char *purl = "www.baidu.com";
//    curl_easy_setopt(curl, CURLOPT_URL, purl);  
// 
//    std::string strFileName = "/root/testpic/123.data";
//    
//    if((fp = fopen(strFileName.c_str(),"w")) == NULL)
//    {
//        curl_easy_cleanup(curl);
//        exit(1);
//    }
//  //CURLOPT_WRITEFUNCTION 将后继的动作交给write_data函数处理
//    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);  
//    curl_easy_perform(curl);
//    curl_easy_cleanup(curl);
//    exit(0);
//    return 0;
    
    
    
    CHttpClient client;
    
    // int Post(const std::string & strUrl, const std::string & strPost, std::string & strResponse);
    std::string  strUrl         = "http://192.168.1.228:8080/ipCar/is/carPassLog";
    std::string  strPost        = "";
    std::string  strResponse    = "";
    client.Post(strUrl, strPost, strResponse);
    
    printf("%s\n", strResponse.c_str());
    
    
    
    
    
    
}

