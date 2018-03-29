//2013-07-05
//V2.3.0
#ifndef _cy_sys_cy_stream_NetCamera_Stream2_h_
#define _cy_sys_cy_stream_NetCamera_Stream2_h_
#define IS28181 1
#include <string>
#ifdef _WIN32
#ifndef NETCAMERA_STREAM2_EXPORTS
#define NC_STREAM_EXT    __declspec(dllimport)
#else
#define NC_STREAM_EXT    __declspec(dllexport)
#endif
#else
#define NC_STREAM_EXT
#define IN
#define OUT
#endif


typedef void (*Callback_RecvData) (const char *m_ip,void* pImage,int nLength,void* pUser1);

namespace cy_sys
{
    namespace cy_stream
    {
        class NC_STREAM_EXT NetCamera_Stream2
        {
        private:
            void* prtspClient;

        public:
            NetCamera_Stream2(void);
            virtual ~NetCamera_Stream2(void);

            unsigned char* getSPS(unsigned int* len, char * m_ip);
            
            unsigned char* getPPS(unsigned int* len, char * m_ip);
        
            /**
             * @brief ������
             * @param t--0��ʾUDP 1��ʾTCP
             * @param m_autoconn 1--�Զ�����(����ʧ�ܺ������) 0--�ֶ�����
             * @return -1--����ʧ�� 1--���ӳɹ�
            */
            int open(char* server_ip,int m_transtype = 0 ,int m_autoconn = 1,unsigned int time_Out = 5);
            
            /**
             * @brief �ر�����
             * @param 
             * @return 
            */
            void Close(void);

            /**
             * @brief �ر�ָ��IP����
             * @param 
             * @return 
            */
            void Close(char * m_ip);


            /**
             * @brief ����h264���ݽ��ջص��������������ûص�������open����ǰ����
             * @param pFun--�ص�����ָ��
             * @param pUser1--�û�����1
             * @param pUser2--�û�����2
             * @return ��
            */
            void SetCallBack(Callback_RecvData pFun,void* pUser1);


            /**
             * @brief ���1֡h264���ݣ�����������
             * @return �������ݳ��� 0��ʾû������
            */
            unsigned getH264Data(unsigned char* buf,char * m_ip);
        };
    }
}
#endif


