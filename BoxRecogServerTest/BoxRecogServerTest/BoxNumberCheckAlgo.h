/* 
 * File:   BoxNumberCheckAlgo.h
 * Author: root
 *
 * Created on 2015��1��28��, ����1:56
 */

#ifndef BOXNUMBERCHECKALGO_H
#define	BOXNUMBERCHECKALGO_H
#include <string>

class BoxNumberCheckAlgo {
public:
    BoxNumberCheckAlgo();
    BoxNumberCheckAlgo(const BoxNumberCheckAlgo& orig);
    virtual ~BoxNumberCheckAlgo();
private:
    
public:
    int GetBoxNumCheckbit(const std::string &strBoxNum, char &chVerifyCode);

};

#endif	/* BOXNUMBERCHECKALGO_H */

