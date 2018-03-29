/* 
 * File:   opensslSigner.cpp
 * Author: root
 * 
 * Created on 2017年1月13日, 上午11:49
 */

#include "opensslSigner.h"
#include <openssl/ossl_typ.h>
#include <string.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/evp.h>


opensslSigner::opensslSigner()
{
}

opensslSigner::opensslSigner(const opensslSigner& orig)
{
}

opensslSigner::~opensslSigner()
{
}


void opensslSigner::sign(EVP_PKEY* evpKey,BYTE** signValue,unsigned int &signLen,BYTE* text,int textLen)  
{  
    
    
    
//    EVP_MD_CTX mdctx;   //摘要算法上下文变量     
//       
//    if(evpKey == NULL)    
//    {    
//        printf("EVP_PKEY_new err\n");    
//        return;    
//    }    
//      
//    //以下是计算签名的代码     
//    EVP_MD_CTX_init(&mdctx);        //初始化摘要上下文     
//    if(!EVP_SignInit_ex(&mdctx,EVP_md5(),NULL)) //签名初始化，设置摘要算法     
//    {    
//        printf("err\n");    
//        EVP_PKEY_free(evpKey);    
//        return;    
//    }    
//    if(!EVP_SignUpdate(&mdctx,text,textLen)) //计算签名（摘要）Update     
//    {    
//        printf("err\n");    
//        EVP_PKEY_free(evpKey);    
//        return;    
//    }    
//    if(!EVP_SignFinal(&mdctx,*signValue,&signLen,evpKey))  //签名输出     
//    {    
//        printf("err\n");    
//        EVP_PKEY_free(evpKey);    
//        return;    
//    }    
//    printf("消息\"%s\"的签名值是：\n",text);    
//    //printByte(*signValue,signLen);  
//    printf("\n");    
//    EVP_MD_CTX_cleanup(&mdctx);    
  
} 

void opensslSigner::verify(EVP_PKEY* evpKey,BYTE* text,unsigned int textLen,BYTE* signValue,unsigned int signLen)  
{  
//    ERR_load_EVP_strings();  
//    EVP_MD_CTX mdctx;   //摘要算法上下文变量     
//    EVP_MD_CTX_init(&mdctx);    //初始化摘要上下文     
//    if(!EVP_VerifyInit_ex(&mdctx, EVP_md5(), NULL)) //验证初始化，设置摘要算法，一定要和签名一致     
//    {    
//        printf("EVP_VerifyInit_ex err\n");    
//        EVP_PKEY_free(evpKey);    
//        return;    
//    }    
//    if(!EVP_VerifyUpdate(&mdctx, text, textLen)) //验证签名（摘要）Update     
//    {    
//        printf("err\n");    
//        EVP_PKEY_free(evpKey);    
//        return;    
//    }    
//    if(!EVP_VerifyFinal(&mdctx,signValue,signLen,evpKey))    
//    {    
//        printf("verify err\n");    
//        EVP_PKEY_free(evpKey);    
//        EVP_MD_CTX_cleanup(&mdctx);    
//        return;  
//    }    
//    else    
//    {    
//        printf("验证签名正确.\n");    
//    }    
//    //释放内存     
//    EVP_PKEY_free(evpKey);    
//    EVP_MD_CTX_cleanup(&mdctx);    
}

