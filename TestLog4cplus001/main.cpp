/* 
 * File:   main.cpp
 * Author: root
 *
 * Created on 2017年8月31日, 下午4:02
 */

#include <cstdlib>
#include <iostream>  
#include <log4cplus/logger.h>  
#include <log4cplus/loggingmacros.h>  
#include <log4cplus/configurator.h>  
#include <iomanip>  
#include <log4cplus/logger.h>  
#include <log4cplus/fileappender.h>  
#include <log4cplus/consoleappender.h>  
#include <log4cplus/layout.h>  
#include <time.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  

using namespace log4cplus;
using namespace log4cplus::helpers;

using namespace std;

void test1()
{
    SharedAppenderPtr appendPtr(new RollingFileAppender("log_text.txt",1*1024*1024,1,true));  
    Logger logger = Logger::getInstance("test1");  
    std::string pattern = "%D{%Y-%m-%d %H:%M:%S.%Q}|%-5p|%c[2]|%t|%F:%L|%m%n";  
    std::auto_ptr<Layout> layout_(new PatternLayout(pattern));  
    appendPtr->setLayout(layout_);  
    logger.addAppender(appendPtr);  
    logger.setLogLevel(ALL_LOG_LEVEL);  
    LOG4CPLUS_INFO(logger,  "Info message");  
    LOG4CPLUS_DEBUG(logger, "Debug message");  
    LOG4CPLUS_TRACE(logger, "Trace message");  
    LOG4CPLUS_ERROR(logger, "Error message");  
    LOG4CPLUS_WARN(logger,  "Warning message");  
    LOG4CPLUS_FATAL(logger, "fatal message");      
}

void test2()
{
    SharedAppenderPtr appendPtr(new RollingFileAppender("log_text.txt",1*1024*1024,1,true));  
    Logger logger = Logger::getInstance("test2");  
    std::string pattern = "%D{%Y-%m-%d %H:%M:%S.%Q}|%-5p|%c[2]|%t|%F:%L|%m%n";  
    std::auto_ptr<Layout> layout_(new PatternLayout(pattern));  
    appendPtr->setLayout(layout_);  
    logger.addAppender(appendPtr);  
    logger.setLogLevel(ALL_LOG_LEVEL);  
    LOG4CPLUS_INFO(logger,  "Info message");  
    LOG4CPLUS_DEBUG(logger, "Debug message");  
    LOG4CPLUS_TRACE(logger, "Trace message");  
    LOG4CPLUS_ERROR(logger, "Error message");  
    LOG4CPLUS_WARN(logger,  "Warning message");  
    LOG4CPLUS_FATAL(logger, "fatal message");   
}

/*
 * 
 */
int main(int argc, char** argv)
{
    SharedAppenderPtr appendPtr(new RollingFileAppender("log_text.txt",200*1024,3,true));  
    Logger logger = Logger::getInstance("test");  
    std::string pattern = "%D{%Y-%m-%d %H:%M:%S.%Q}|%-5p|%c[2]|%t|%F:%L|%m%n";  
    std::auto_ptr<Layout> layout_(new PatternLayout(pattern));  
    appendPtr->setLayout(layout_);  
    logger.addAppender(appendPtr);  
    logger.setLogLevel(ALL_LOG_LEVEL);  
    LOG4CPLUS_INFO(logger,  "Info message");  
    LOG4CPLUS_DEBUG(logger, "Debug message");  
    LOG4CPLUS_TRACE(logger, "Trace message");  
    LOG4CPLUS_ERROR(logger, "Error message");  
    LOG4CPLUS_WARN(logger,  "Warning message");  
    LOG4CPLUS_FATAL(logger, "fatal message");  
    
    test1();
    test2();
    return 0;
}

