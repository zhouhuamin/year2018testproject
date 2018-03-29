/* 
 * File:   UtilTools.h
 * Author: root
 *
 * Created on 2015��8��1��, ����5:14
 */

#ifndef UTILTOOLS_H
#define	UTILTOOLS_H

#include <iostream>
#include <string>
#include <errno.h>
#include <iconv.h>

using namespace std;

//Linux�� GB2312��UTF8ת���ӿ�
class CUtilTools  
{
public:
    CUtilTools(){};
    ~CUtilTools(){};
    
    //iInLen�ĳ��Ȳ�����\0��Ӧ����strlen������ֵ�Ǵ�����sOut����
    static int Utf8ToGb2312(char *sOut, int iMaxOutLen, const char *sIn, int iInLen)
    {
        char *pIn = (char *)sIn;
        char *pOut = sOut;
        size_t ret;
        size_t iLeftLen=iMaxOutLen;
        iconv_t cd;

        cd = iconv_open("gb2312", "utf-8");
        if (cd == (iconv_t) - 1)
        {
            return -1;
        }
        size_t iSrcLen=iInLen;
        ret = iconv(cd, &pIn,&iSrcLen, &pOut,&iLeftLen);
        if (ret == (size_t) - 1)
        {
            //printf("%s\n", strerror(errno));
            iconv_close(cd);
            return -1;
        }

        iconv_close(cd);

        return (iMaxOutLen - iLeftLen);
    }
    
    //iInLen�ĳ��Ȳ�����\0��Ӧ����strlen������ֵ�Ǵ�����sOut����
    static int Gb2312ToUtf8(char *sOut, int iMaxOutLen, const char *sIn, int iInLen)
    {
        char *pIn = (char *)sIn;
        char *pOut = sOut;
        size_t ret;
        size_t iLeftLen=iMaxOutLen;
        iconv_t cd;

        cd = iconv_open("utf-8", "gb2312");
        //cd = iconv_open("utf-8", "iso-8859-1");
        if (cd == (iconv_t) - 1)
        {
            return -1;
        }
        size_t iSrcLen=iInLen;
        ret = iconv(cd, &pIn,&iSrcLen, &pOut,&iLeftLen);
        if (ret == (size_t) - 1)
        {
            iconv_close(cd);
            return -1;
        }

        iconv_close(cd);

        return (iMaxOutLen - iLeftLen);
    }     
};


//int main(int argc, char* argv[])
//{
//    char* pszOri = "�����ַ�����";
//    cout << "strlen:" << strlen(pszOri) << endl;
//    
//    char pszDst[50] = {0};
//    
//    int iLen = CUtilTools::Gb2312ToUtf8(pszDst, 50, pszOri, strlen(pszOri)); // Gb2312ToUtf8
//    
//    cout << iLen << "," << strerror(errno) << "," << pszDst << endl;
//    
//    cout << "-----------" << endl;
//    
//    char pszGbDst[50] = {0};  
//    int iNewLen = CUtilTools::Utf8ToGb2312(pszGbDst, 50, pszDst, iLen); // Utf8ToGb2312   
//    cout << iNewLen << "," << strerror(errno) << "," << pszGbDst << endl;
//    
//    
////    ���Ϊ��
////    size:12
////    18,Success,�??�??�??
////    -----------
////    12,Success,�����ַ�����
////    
////    //���Կ�����UTF8��ʽ�£�һ�������ַ�ռ�����ֽڣ���GB2312��ռ�����ֽڡ�
//    
//    return 0;
//}

#endif	/* UTILTOOLS_H */

