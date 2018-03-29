#ifndef RECO_CLIENT_SDK_H_
#define RECO_CLIENT_SDK_H_

//集装箱号区域

struct JZ_Region {
    int x;
    int y;
    int width;
    int height;
};
//颜色

enum JZ_Color {
    RED, YELLOW,BLUE, WHITE, GRAY, GREEN, OTHER
};

//集装箱号排列方式

enum JZ_AlignType {
    H_Align, T_Align
}; //H:horizontal 横排  T：tandem 竖排

struct JZ_Align {
    JZ_AlignType Atype; //排列方式 横排或者竖排
    int count; //排数
};

struct JZ_RecoContaID {
    int  nResult;
    char ContaID[12]; //集装箱号：4位字母，6位数字，1位校验码
    char Type[5]; //集装箱类型
    float fAccuracy;
    JZ_Region IDReg; //区域
    JZ_Region TypeReg; //区域
    JZ_Color color; //颜色
    JZ_Align ali; //排列方式
    
};

typedef void (*_RECO_RESULT_CALLBACK)(char* szRecoSequence,JZ_RecoContaID* pRecoREsult);

extern "C" int RecoInit(char* szRecoServerIP,int nRecoServerPort);
extern "C" int ContaReco(char* szRecoSequence, char *ImagePath);
extern "C" int Release();
extern "C" int SetRecoResultCallback(_RECO_RESULT_CALLBACK pRecoResultCallbase);

#endif
