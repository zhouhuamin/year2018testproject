/* 
 * File:   rsaPair.h
 * Author: root
 *
 * Created on 2017年1月13日, 下午12:05
 */

#ifndef RSAPAIR_H
#define	RSAPAIR_H
#include <iostream>
#include <string>
#include <sstream>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/bn.h> // this is for the BN_new

class rsaPair {
public:
    rsaPair();
    rsaPair(const rsaPair& orig);
     rsaPair ( std::string pub_k_path , std::string pri_key_path, std::string psword ) ;
    virtual ~rsaPair();
private:
    
public:
    int create_key_pair () ;

  private :
    std::string pub_key_path ;
    std::string pri_key_path ;
    std::string password ;
    RSA *pub_key ;
    RSA *pri_key ;
};

#endif	/* RSAPAIR_H */

