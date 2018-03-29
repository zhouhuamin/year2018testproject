//2013-07-05
//V2.3.0

#ifndef _NETCAMERA_SNAPSERVER_H_
#define _NETCAMERA_SNAPSERVER_H_

#ifndef _WIN32	//linux
#include <pthread.h>
#include <assert.h>
#define SOCKET int
typedef pthread_t HANDLE ;
#define NC_SNAP_EXT
#ifndef _CRI_SEC_
#define _CRI_SEC_
#define CRITICAL_SECTION pthread_mutex_t
#define InitializeCriticalSection(x) pthread_mutex_init(x,NULL)
#define DeleteCriticalSection(x) pthread_mutex_destroy(x)
#define EnterCriticalSection(x) pthread_mutex_lock(x)
#define LeaveCriticalSection(x) pthread_mutex_unlock(x)
#endif
#else		//windows
#define _CRTDBG_MAP_ALLOC
#include<stdlib.h>
#include<windows.h>
#ifndef NETCAMERA_SNAPSERVER_EXPORTS
#define NC_SNAP_EXT	__declspec(dllimport)
#else
#define NC_SNAP_EXT	__declspec(dllexport)
#endif
#endif
#include <algorithm>
#include <queue>
#include <list>


////////////////////////////////////////////////////////////////////////////////
//STRUCT DEFINE
#ifndef _FRAME_HEADER_
#define _FRAME_HEADER_
struct FRAME_HEADER
{
	unsigned short w;							// ͼ���
	unsigned short h;							// ͼ���	
	unsigned short bits;						// ͼ��λ�� 8~16bits
	unsigned short format;					    // ͼ�����ʹ���(0--�Ҷȣ�1--Bayer_RG��2--Bayer_GR��3--Bayer_BG��5--RGB��6--YUV422��7--JPEG, 8--MPEG4 9--h264)	
	unsigned short frame_type;				    // ֡����(0--��ͨ��1--ץ��ͼ��2--����֡)
	unsigned short frame_rev;					// ����	
	unsigned int  firmware_version;			    // �̼�����汾
	unsigned int  device_no;					// �豸���	
	unsigned int  len;						    // ͼ�����ݳ���	
	unsigned int  speed;						// ˫��Ȧ����ֵ(us)
	unsigned int  rs232;						// ������Ϣ(1~4�ֽ�)
	unsigned short year;						// ͼ��ɼ�ʱ��
	unsigned short month;
	unsigned short day;
	unsigned short hour;
	unsigned short minute;
	unsigned short second;	
	unsigned int ip;							// �ɼ���ǰ֡�������IP
	unsigned int frame_count;					// �ܹ���ץ��֡��Ŀ
	unsigned int trigger_count;				// �ܹ��Ĵ�����
	unsigned int trigger_index;				// ����������
	unsigned int frame_no;					// ֡��	
	unsigned int gain;						// ��ǰץ�Ĳ���
	unsigned int time;						// �ع�ʱ��
	unsigned int gain_r;						// ������
	unsigned int gain_g;						// ������
	unsigned int gain_b;						// ������
	unsigned int mode;						// ���������ģʽ
	unsigned int JpegQ;						// JPEGѹ��Ʒ��
	unsigned int td1;							// ץ����ʱ(��λus)
	unsigned int td2;							// �ع���ʱ(��λus)
	unsigned int trig_chl;					// ����ͨ��
	unsigned int msecond;						// ms��ʱ
	unsigned int yavg;						// ƽ������
	unsigned int vid_head;					// ��ý��ؼ�֡
	unsigned int st_usr_param;				// ������Ϣ
	unsigned int red_time;					// ��Ƽ�ʱ
	//unsigned long rev[6];					// ��������
	unsigned int grey;						// һ�����ԭʼ����
	unsigned int wz_type;						// Υ�����ͣ�d0-����,d1-�����
	unsigned short traffic_code;				// Υ�´���
	unsigned char capture_no;					// ���δ���ץ��������total��
	unsigned char rev_1;						// ����λ
	unsigned int rev[3];						// ��������
	unsigned char user_info[64];				// �û�����
};
#endif
typedef struct _CapClientStruct
{
    HANDLE m_hCapThread;
    char m_strIp[16];
    SOCKET m_sockCap;
}CapClientStruct;
class NC_SNAP_EXT NetCamera_SnapServer  
{
	SOCKET listen_s; //�������׽���
public:
    NetCamera_SnapServer();
    virtual ~NetCamera_SnapServer();
public:
    class Cap_Lock
    {
        CRITICAL_SECTION        m_cs;
    public:
        Cap_Lock();
        ~Cap_Lock();
    public:
        int Lock();
        int UnLock();
    };
public:
	SOCKET getListenS(void){return listen_s;}
	void setListenS(SOCKET s){listen_s=s;}
	int &getState(void){return m_nState;}
	int StartRecv(unsigned short port=9999);
	int GetImage(unsigned char* buf);
	unsigned long GetImageSize();
	int AddImage(unsigned char* pImage,int buflen);
    unsigned short GetRecvPort();
private:
	int						m_nState;
	int						m_nMaxQueueLength;
	unsigned short			m_nPort;
    HANDLE m_hCapThread;
	unsigned char* pImgData;
	int queIndex;
	std::queue<unsigned char*>*	m_pImageQueue;
	CRITICAL_SECTION		m_cs;
public:
	static int MySend(SOCKET& s, char *b, int l,int to);
	static int MyRecv(SOCKET s, char *b, int l, int to);
private:
	static int MySendWithTimeout(SOCKET s, char *b, int l, int to, int *send_len);
	static int MyRecvWithTimeout(SOCKET s, char *b, int l, int to, int *recv_len);
};

#endif
