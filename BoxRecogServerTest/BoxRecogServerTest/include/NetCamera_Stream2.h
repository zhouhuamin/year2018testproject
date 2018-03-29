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
             * @brief 打开连接
             * @param t--0表示UDP 1表示TCP
             * @param m_autoconn 1--自动连接(连接失败后会重连) 0--手动连接
             * @return -1--连接失败 1--连接成功
            */
            int open(char* server_ip,int m_transtype = 0 ,int m_autoconn = 1,unsigned int time_Out = 5);
            
            /**
             * @brief 关闭连接
             * @param 
             * @return 
            */
            void Close(void);

            /**
             * @brief 关闭指定IP连接
             * @param 
             * @return 
            */
            void Close(char * m_ip);


            /**
             * @brief 设置h264数据接收回调函数，如需利用回调，请在open函数前调用
             * @param pFun--回调函数指针
             * @param pUser1--用户数据1
             * @param pUser2--用户数据2
             * @return 无
            */
            void SetCallBack(Callback_RecvData pFun,void* pUser1);


            /**
             * @brief 获得1帧h264数据，非阻塞调用
             * @return 返回数据长度 0表示没有数据
            */
            unsigned getH264Data(unsigned char* buf,char * m_ip);
        };
    }
}
#endif


