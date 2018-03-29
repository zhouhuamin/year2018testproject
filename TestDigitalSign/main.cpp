/* 
 * File:   main.cpp
 * Author: root
 *
 * Created on 2017年1月13日, 上午11:32
 */

#include <cstdlib>
#include <string.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include "rsaPair.h"
#include <openssl/sha.h>
#include <cassert>  
using namespace std;


//加密  
std::string EncodeRSAKeyFile( const std::string& strPemFileName, const std::string &strData)  
{  
    if (strPemFileName.empty() || strData.empty())  
    {  
        assert(false);  
        return "";  
    }  
    FILE* hPriKeyFile = fopen(strPemFileName.c_str(), "rb");  
    if( hPriKeyFile == NULL )  
    {  
        assert(false);  
        return "";   
    }  
    std::string strRet;  
    RSA* pRSAPriKey = RSA_new();  
            
            
            
    if(PEM_read_RSAPrivateKey(hPriKeyFile, &pRSAPriKey, 0, 0) == NULL)  
    {  
        fclose(hPriKeyFile);  
        assert(false);  
        return "";  
    }  
  
    int nLen = RSA_size(pRSAPriKey);  
    char* pEncode = new char[nLen + 1];  
    int ret = RSA_private_encrypt(strData.size(), (const unsigned char*)strData.c_str(), (unsigned char*)pEncode, pRSAPriKey, RSA_PKCS1_PADDING);  
    if (ret >= 0)  
    {  
        strRet = std::string(pEncode, ret);  
    }  
    delete[] pEncode;  
    RSA_free(pRSAPriKey);  
    fclose(hPriKeyFile);  
    CRYPTO_cleanup_all_ex_data();   
    return strRet;  
} 


//加密  
std::string EncodeRSAKeyFile1( const std::string& strPemFileName, const std::string& strData)  
{  
    if (strPemFileName.empty())  
    {  
        assert(false);  
        return "";  
    }  
    FILE* hPubKeyFile = fopen(strPemFileName.c_str(), "rb");  
    if( hPubKeyFile == NULL )  
    {  
        assert(false);  
        return "";   
    }  
    std::string strRet;  
    RSA* pRSAPublicKey = RSA_new();  
            
            
            
    if(PEM_read_RSA_PUBKEY(hPubKeyFile, &pRSAPublicKey, 0, 0) == NULL)  
    {  
        fclose(hPubKeyFile);  
        assert(false);  
        return "";  
    }  
  
    int nLen = RSA_size(pRSAPublicKey);  
    char* pEncode = new char[nLen + 1];  
    int ret = RSA_public_encrypt(strData.length(), (const unsigned char*)strData.c_str(), (unsigned char*)pEncode, pRSAPublicKey, RSA_PKCS1_PADDING);  
    if (ret >= 0)  
    {  
        strRet = std::string(pEncode, ret);  
    }  
    delete[] pEncode;  
    RSA_free(pRSAPublicKey);  
    fclose(hPubKeyFile);  
    CRYPTO_cleanup_all_ex_data();   
    return strRet;  
}  
  
//解密  
std::string DecodeRSAKeyFile( const std::string& strPemFileName, const std::string& strData )  
{  
    if (strPemFileName.empty() || strData.empty())  
    {  
        assert(false);  
        return "";  
    }  
    FILE* hPubKeyFile = fopen(strPemFileName.c_str(),"rb");  
    if( hPubKeyFile == NULL )  
    {  
        assert(false);  
        return "";  
    }  
    std::string strRet;  
    RSA* pRSAPublicKey = RSA_new();  
    if(PEM_read_RSA_PUBKEY(hPubKeyFile, &pRSAPublicKey, 0, 0) == NULL)  
    {  
        assert(false);  
        return "";  
    }  
    int nLen = RSA_size(pRSAPublicKey);  
    char* pDecode = new char[nLen+1];  
  
    int ret = RSA_public_decrypt(strData.length(), (const unsigned char*)strData.c_str(), (unsigned char*)pDecode, pRSAPublicKey, RSA_PKCS1_PADDING);  
    if(ret >= 0)  
    {  
        strRet = std::string((char*)pDecode, ret);  
    }  
    delete [] pDecode;  
    RSA_free(pRSAPublicKey);  
    fclose(hPubKeyFile);  
    CRYPTO_cleanup_all_ex_data();   
    return strRet;  
} 




//解密  
std::string DecodeRSAKeyFile1( const std::string& strPemFileName, const std::string& strData )  
{  
    if (strPemFileName.empty() || strData.empty())  
    {  
        assert(false);  
        return "";  
    }  
    FILE* hPriKeyFile = fopen(strPemFileName.c_str(),"rb");  
    if( hPriKeyFile == NULL )  
    {  
        assert(false);  
        return "";  
    }  
    std::string strRet;  
    RSA* pRSAPriKey = RSA_new();  
    if(PEM_read_RSAPrivateKey(hPriKeyFile, &pRSAPriKey, 0, 0) == NULL)  
    {  
        assert(false);  
        return "";  
    }  
    int nLen = RSA_size(pRSAPriKey);  
    char* pDecode = new char[nLen+1];  
  
    int ret = RSA_private_decrypt(strData.length(), (const unsigned char*)strData.c_str(), (unsigned char*)pDecode, pRSAPriKey, RSA_PKCS1_PADDING);  
    if(ret >= 0)  
    {  
        strRet = std::string((char*)pDecode, ret);  
    }  
    delete [] pDecode;  
    RSA_free(pRSAPriKey);  
    fclose(hPriKeyFile);  
    CRYPTO_cleanup_all_ex_data();   
    return strRet;  
} 
/*
 * 
 */
int main(int argc, char** argv)
{
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
    
    SHA_CTX stx;
    FILE *fp = NULL;
    std::string filename = "/root/zhouhm/myopenssl/test/GatherInfo.xml";
    
    fp = fopen(filename.c_str(), "rb");
    if (fp == NULL)
    {
        return -1;
    }
    
    int len = 0;
    char buffer[1024] = {0};
    unsigned char outmd[20] = {0};
    while((len=fread(buffer,1,1024,fp))>0)
    {
        SHA1_Update(&stx,buffer,len);
        memset(buffer,0,sizeof(buffer));
    }   
    fclose(fp);
    SHA1_Final(outmd, &stx);
    
    std::string strData = "";
    char szXX[2 + 1] = {0};
    for(int i=0; i<20; i<i++)
    {
        sprintf(szXX, "%02X", outmd[i]);
        if (i != 19)
        {
            strData += szXX;
            strData += "-";
        }
        else
        {
            strData += szXX;
        }
    } 
    
    
    
    
    
  
    //密文（二进制数据）  
    string two = EncodeRSAKeyFile("/root/zhouhm/myopenssl/test/aprikey.pem", strData); 
    //string two = EncodeRSAKeyFile("/root/zhouhm/myopenssl/test/b-to-pub.pem", one); 
    cout << "two: " << two << endl;  
  
    //顺利的话，解密后的文字和原文是一致的  
    string three = DecodeRSAKeyFile("/root/zhouhm/myopenssl/test/apubkey.pem", two);  
    //b-to-pri.pem
    //string three = DecodeRSAKeyFile("/root/zhouhm/myopenssl/test/b-to-pri.pem", two);
    
    cout << "three: " << three << endl;   




    
    
    
    
    
//        SHA_CTX ctx;
//    unsigned char outmd[20];
//    int i=0;
//
//    memset(outmd,0,sizeof(outmd));
//    SHA1_Init(&ctx);
//    SHA1_Update(&ctx,"hel",3);
//    SHA1_Update(&ctx,"lo\n",3);
//    SHA1_Final(outmd,&ctx);
//    for(i=0;i<20;i<i++)
//    {
//        printf("%02X",outmd[i]);
//    }
//    printf("\n");
    
    
    
//string pub_key_path , pri_key_path ;
//  string password ;
//  
//  cout << "public key path " << endl;
//  cin >> pub_key_path ;
//  cout << pub_key_path << endl ; 
//
//
//  cout << "private key path " << endl ;
//  cin >> pri_key_path ;
//  cout << pri_key_path <<endl ;
//  
//
//  cout << "password " << endl ;
//  cin >> password ;
//  cout << password <<endl;
//
//  
// // rsaPair key_pair ;
//  rsaPair key_pair ( pub_key_path , pri_key_path , password) ; 
//  
//    key_pair.create_key_pair () ;     
    
//    //原文  
//    const string one = "skl;dfhas;lkdfhslk;dfhsidfhoiehrfoishfsidf";  
//    cout << "one: " << one << endl;  
//  
//    //密文（二进制数据）  
//    string two = EncodeRSAKeyFile("/root/zhouhm/myopenssl/test/apubkey.pem", one); 
//    //string two = EncodeRSAKeyFile("/root/zhouhm/myopenssl/test/b-to-pub.pem", one); 
//    cout << "two: " << two << endl;  
//  
//    //顺利的话，解密后的文字和原文是一致的  
//    string three = DecodeRSAKeyFile("/root/zhouhm/myopenssl/test/aprikey.pem", two);  
//    //b-to-pri.pem
//    //string three = DecodeRSAKeyFile("/root/zhouhm/myopenssl/test/b-to-pri.pem", two);
//    
//    cout << "three: " << three << endl;      
    return 0;
}

