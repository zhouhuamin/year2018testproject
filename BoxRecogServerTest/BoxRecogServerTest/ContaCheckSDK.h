#ifndef CONTA_CHECK_SDK_H_
#define CONTA_CHECK_SDK_H_

#include <list>
#include <vector>
#include <map>
#include <string>

#define MAX_IMAGE_COUNT (256)

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

    
struct structContaOwnerData
{
    int nLineNo;
    std::string strCompanyCode;
    std::string strCompanyName;
};
    
    
typedef enum jz_enumBoxNumberDirection
{
    FRONT       = 0,
    FRONTLEFT   = 1,
    FRONTRIGHT  = 2,
    BACK        = 3,
    BACKLEFT    = 4,
    BACKRIGHT   = 5
}BoxNumberDirection;

struct structBoxNumberRecogResult
{
    std::string strSeqNo;
    std::string strPicKey;
    std::string strBoxNumber;
    std::string strBoxModel;
    std::string strBoxColor;
    std::string strArrangement;
    BoxNumberDirection  direct;
    int                 nAccuracy;
    structBoxNumberRecogResult()
    {
        direct    = FRONT;
        nAccuracy = 0;
    }
};

struct structBoxNumberRecogResultCorrected
{
    int nBoxType;                   // 1:?¿ç??   2:??ç®?  3:??ç®? 4:??ç®?
    int nPicNumber;
    std::string strFrontBoxNumber;
    std::string strBackBoxNumber;
    std::string strFrontBoxModel;
    std::string strBackBoxModel;
    std::string strRecogResult;
    std::string CONTA_PIC_F;
    std::string CONTA_PIC_B;
    std::string CONTA_PIC_LF;
    std::string CONTA_PIC_RF;
    std::string CONTA_PIC_LB;
    std::string CONTA_PIC_RB;
};
    //ÏäºÅÐ£Ñé
    int ContaCheck(std::vector<std::string> &statusVect, const std::vector<structBoxNumberRecogResult> &boxNumberSet, structBoxNumberRecogResultCorrected &resultCorrected);
    
#ifdef __cplusplus
}
#endif

#endif
