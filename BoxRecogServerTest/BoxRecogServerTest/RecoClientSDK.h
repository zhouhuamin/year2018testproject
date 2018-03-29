#ifndef RECO_CLIENT_SDK_H_
#define RECO_CLIENT_SDK_H_

//��װ�������

struct JZ_Region {
    int x;
    int y;
    int width;
    int height;
};
//��ɫ

enum JZ_Color {
    RED, YELLOW,BLUE, WHITE, GRAY, GREEN, OTHER
};

//��װ������з�ʽ

enum JZ_AlignType {
    H_Align, T_Align
}; //H:horizontal ����  T��tandem ����

struct JZ_Align {
    JZ_AlignType Atype; //���з�ʽ ���Ż�������
    int count; //����
};

struct JZ_RecoContaID {
    int  nResult;
    char ContaID[12]; //��װ��ţ�4λ��ĸ��6λ���֣�1λУ����
    char Type[5]; //��װ������
    float fAccuracy;
    JZ_Region IDReg; //����
    JZ_Region TypeReg; //����
    JZ_Color color; //��ɫ
    JZ_Align ali; //���з�ʽ
    
};

typedef void (*_RECO_RESULT_CALLBACK)(char* szRecoSequence,JZ_RecoContaID* pRecoREsult);

extern "C" int RecoInit(char* szRecoServerIP,int nRecoServerPort);
extern "C" int ContaReco(char* szRecoSequence, char *ImagePath);
extern "C" int Release();
extern "C" int SetRecoResultCallback(_RECO_RESULT_CALLBACK pRecoResultCallbase);

#endif
