// TopSkyIPC.h: interface for the CTopSkyIPC class.
//
//////////////////////////////////////////////////////////////////////
#if !defined _TOP_SKY_IPC_H
#define _TOP_SKY_IPC_H

#include "../include/BaseDevice.h"
#include "../include/MutexGuard.h"

#include "../ToolLib/FreeLongLog.h"
#include "../ToolLib/MRSWInt.h"


#define MAX_PIC_BUFFER    (16)

struct VP_PASS_VEHICLE_BUFFER
{
    char szMemBuffer[1024 * 1024 * 2];
};

class CMHICReader : public CBaseDevice
{
public:
    CMHICReader();
    ~CMHICReader();
public:

    //device interface
    int init_device(DeviceInfo device_info);
    int get_module_info(ModuleInfo& moduleinfo);

    int start_work();

    int pause(bool is_stop);

    int stop_work();

    int read_data();

    int begin_test_device();

    int end_test_device();

    int AutoDectectDevice();

    int SetRunState();

    int reset_device();

    int get_device_state();

    int set_device_state();

    int close_device();

    int begin_test();

    int end_test();

    int SetReadDataCallback(_READ_DATA_CALLBACK_ pReadDataCallback,void* pUserData);

    int SetErrorStateCallback(_ERROR_STATE_CALLBACK_ pErrorStateCallback,void* pUserData);

    //CONTROL INTERFACE
    int led_show_pic(char* pic_screen_code);

    int led_show_text(char* screen_text);

    int play_sound(char* sound_code);

    int give_alarm(int time_span);

    int lcd_show_text(char* conta_idf, char* conta_idb, char* conta_damage_f, char* conta_damage_b);

    int print_ticket(char* chartoprint);


    //IO PORT INTERFACE
    unsigned char ReadPortByteData(unsigned short wChannel);

    unsigned char WritePortByteData(unsigned short wChannel, unsigned char wData);

    unsigned char WritePortDwordData(unsigned short wChannel, unsigned int dwData);

    unsigned char ReadPortDwordData(unsigned short wChannel);

    unsigned char WriteIOBit(unsigned short wChannel, unsigned short wBit, unsigned short wState);
private:
    int GetProcessPath(char* cpPath);
    int init_reader();
private:
    DeviceInfo   reader_device_info;
    ModuleInfo  module_info;
    CFreeLongLog* m_pLog;

    bool            is_init_reader;
    bool            is_thread_init;

public:
    CMRSWBool is_work_now;
    CMRSWBool is_pause_now;

    int          reader_handle;

    unsigned char read_buffer[64];
    unsigned char callback_buffer[64];
    
    _READ_DATA_CALLBACK_ m_pReadDataCallback;
    void*                     m_pDataCallbackUserData;

    _ERROR_STATE_CALLBACK_ m_pErrorStateCallback;
    void*                     m_pErrorCallbackData;

};

#endif
