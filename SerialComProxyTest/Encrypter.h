/* 
 * File:   Encrypter.h
 * Author: root
 *
 * Created on 2017��1��13��, ����7:43
 */

#ifndef ENCRYPTER_H
#define	ENCRYPTER_H

#include <string>

class Encrypter {
public:
    Encrypter();
    Encrypter(const Encrypter& orig);
    virtual ~Encrypter();
private:

    
public:
    int EncodeRSAKeyFile(const std::string& strPemFileName, const std::string &strData, char *pOutData, int &nOutLen);
    std::string DecodeRSAKeyFile(const std::string& strPemFileName, char *pInData, int nOutLen);
    
    std::string GetMessageHash(const std::string &strXml);
};

#endif	/* ENCRYPTER_H */

