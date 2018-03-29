/* 
 * File:   opensslSigner.h
 * Author: root
 *
 * Created on 2017年1月13日, 上午11:49
 */

#ifndef OPENSSLSIGNER_H
#define	OPENSSLSIGNER_H

#include <iostream>
#include <string>
#include <sstream>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/bn.h> // this is for the BN_new

typedef unsigned char BYTE;

class opensslSigner {
public:
    opensslSigner();
    opensslSigner(const opensslSigner& orig);
    virtual ~opensslSigner();
private:
    
    
public:
    void sign(EVP_PKEY* evpKey,BYTE** signValue,unsigned int &signLen,BYTE* text,int textLen);
    void verify(EVP_PKEY* evpKey,BYTE* text,unsigned int textLen,BYTE* signValue,unsigned int signLen);

};

#endif	/* OPENSSLSIGNER_H */

