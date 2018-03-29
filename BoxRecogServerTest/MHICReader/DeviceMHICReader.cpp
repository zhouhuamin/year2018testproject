//
//////////////////////////////////////////////////////////////////////
#include "DeviceMHICReader.h"
#include <errno.h>

#include "mwrf.h"
#include <string>
using namespace std;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
/////////////////////////////////////////////////////////////////////

extern "C" {
    typedef void*(*THREADFUNC)(void*);
}

void* ThreadReadDataFunc(void* lParam)
{
    CMHICReader* pICReader = (CMHICReader*) lParam;
    while (1)
    {
        if (!pICReader->is_work_now.Get() || pICReader->is_pause_now.Get())
        {
            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = 500 * 1000;
            select(0, 0, NULL, NULL, &tv);

            continue;
        }

        pICReader->read_data();

        struct timeval tv;
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        select(0, 0, NULL, NULL, &tv);
    }

    return 0;
}

int CMHICReader::GetProcessPath(char* cpPath)
{

    int iCount;

    iCount = readlink("/proc/self/exe", cpPath, 256);

    if (iCount < 0 || iCount >= 256)
    {
        m_pLog->_XGSysLog("********get process absolute path failed,errno:%d !\n", errno);
        return -1;
    }

    cpPath[iCount] = '\0';

    return 0;

}

CMHICReader::CMHICReader()
{
    reader_device_info.com_port = 0;
    reader_device_info.baud_date = 0;

    m_pReadDataCallback = NULL;
    m_pErrorStateCallback = NULL;

    m_pDataCallbackUserData = NULL;
    m_pErrorCallbackData = NULL;

    is_init_reader = false;
    is_thread_init = false;

    reader_handle = -1;

    is_work_now.Set(false);
    is_pause_now.Set(false);

    strcpy(module_info.factory_name, "SZMH");
    strcpy(module_info.module_name, "ICCardReader");


    /*
     *	获得当前绝对路径
     */
    char szFullPath[256]; // MAX_PATH

    GetProcessPath(szFullPath); //得到程序模块名称，全路径
    printf("full path is %s\n", szFullPath);

    string strCurrentpath(szFullPath);
    int nLast = strCurrentpath.find_last_of("/");

    strCurrentpath = strCurrentpath.substr(0, nLast + 1);

    printf("full path is %s\n", strCurrentpath.c_str());

    if (!m_pLog)
    {
        m_pLog = new CFreeLongLog((char*) strCurrentpath.c_str(), "MHReader", 12);
    }
}

CMHICReader::~CMHICReader()
{

}

int CMHICReader::init_device(DeviceInfo device_info)
{
    reader_device_info.com_port = device_info.com_port;
    reader_device_info.baud_date = device_info.baud_date;

    m_pLog->_XGSysLog("init com port %d,baudrate %d\n", reader_device_info.com_port, reader_device_info.baud_date);

    return 0;
}

int CMHICReader::get_module_info(ModuleInfo& moduleinfo)
{
    memcpy(&moduleinfo, &module_info, sizeof (ModuleInfo));
    return 0;
};

int CMHICReader::init_reader()
{
    int status = 0;
    unsigned char card_status[32] = {0};
    int baud_rate = 115200;

    rf_exit(0);
    reader_handle = rf_init("star500x", reader_device_info.baud_date);

    status = rf_get_status(reader_handle, card_status);

    if (reader_handle < 0 || status)
    {
        m_pLog->_XGSysLog("********connect fail!......\n");
        return -1;
    }
    else
    {
        rf_beep(reader_handle, 10);

    }

    unsigned int tag_type = 0;
    status = rf_reset(reader_handle, 5);
    status = rf_request(reader_handle, 1, &tag_type);
    if (status)
    {
        m_pLog->_XGSysLog("********find card fail!......\n");
        return -1;
    }

    unsigned long snr = 0;
    status = rf_anticoll(reader_handle, 0, &snr);
    if (status)
    {
        m_pLog->_XGSysLog("********anti card fail!......\n");
        return -1;
    }

    unsigned char size = 0;
    status = rf_select(reader_handle, snr, &size);
    if (status)
    {
        m_pLog->_XGSysLog("********select card fail!......\n");
        return -1;
    }

    unsigned char key[7];
    memset(key, 0, 7);

    key[0] = 0XA8;
    key[1] = 0X3F;
    key[2] = 0X63;
    key[3] = 0X17;
    key[4] = 0X69;
    key[5] = 0XE2;
    key[6] = 0X00;

    //	a_hex("A83F631769E2",key,12);
    //	m_key.ReleaseBuffer();

    status = rf_load_key(reader_handle, 0, 0, key);
    if (status)
    {
        m_pLog->_XGSysLog("********load key fail!\n");
        return -1;
    }
    status = rf_authentication(reader_handle, 0, 0);
    if (status)
    {
        m_pLog->_XGSysLog("********auth card fail!\n");
        return -1;
    }

    m_pLog->_XGSysLog("init reader succ!\n");
}

int CMHICReader::start_work()
{
    is_work_now.Set(true);

    if (!is_init_reader)
    {
        is_init_reader = true;
        init_reader();

    }

    if (!is_thread_init)
    {
        is_thread_init = true;
        int nThreadErr = 0;
        pthread_t thread_id;
        nThreadErr = pthread_create(&thread_id, NULL, (THREADFUNC) ThreadReadDataFunc, this);

    }

    m_pLog->_XGSysLog("--------start work!\n");
    return 0;
};

int CMHICReader::pause(bool is_stop)
{
    if (is_stop)
    {
        is_pause_now.Set(true);
    }
    else
    {
        is_pause_now.Set(false);
    }
    return 0;
};

int CMHICReader::stop_work()
{
    return 0;
};

int CMHICReader::read_data()
{
    memset(&read_buffer, 0, 64);
    int status = rf_read(reader_handle, 1, read_buffer);

    if (status)
    {
        m_pLog->_XGSysLog("********read data error or no card to read!\n");
        return -1;
    }
    else
    {
        rf_beep(reader_handle, 10);

    }

    memset(&callback_buffer, 0, 64);
    memcpy(callback_buffer, read_buffer, 10);

    m_pLog->_XGSysLog("card serial:%s!\n", callback_buffer);

    if (m_pReadDataCallback && m_pDataCallbackUserData)
    {
        m_pReadDataCallback(callback_buffer, m_pDataCallbackUserData);
    }

    return 0;
};

int CMHICReader::begin_test_device()
{
    return 0;
};

int CMHICReader::end_test_device()
{
    return 0;
};

int CMHICReader::AutoDectectDevice()
{
    return 0;
};

int CMHICReader::SetRunState()
{
    return 0;
};

int CMHICReader::reset_device()
{
    return 0;
};

int CMHICReader::get_device_state()
{
    return 0;
};

int CMHICReader::set_device_state()
{
    return 0;
};

int CMHICReader::close_device()
{
    if (reader_handle != -1)
    {
        rf_exit(reader_handle);
    }

    return 0;
};

int CMHICReader::begin_test()
{
    return 0;
};

int CMHICReader::end_test()
{
    return 0;
};

int CMHICReader::SetReadDataCallback(_READ_DATA_CALLBACK_ pReadDataCallback,void* pUserData)
{
    m_pReadDataCallback = pReadDataCallback;
    m_pDataCallbackUserData = pUserData;

    return 0;
};

int CMHICReader::SetErrorStateCallback(_ERROR_STATE_CALLBACK_ pErrorStateCallback, void* pUserData)
{
    m_pErrorStateCallback = pErrorStateCallback;
    m_pErrorCallbackData = pUserData;
    return 0;
};


//CONTROL INTERFACE

int CMHICReader::led_show_pic(char* pic_screen_code)
{
    return 0;
};

int CMHICReader::led_show_text(char* screen_text)
{
    return 0;
};

int CMHICReader::play_sound(char* sound_code)
{
    return 0;
};

int CMHICReader::give_alarm(int time_span)
{
    return 0;
};

int CMHICReader::lcd_show_text(char* conta_idf, char* conta_idb, char* conta_damage_f, char* conta_damage_b)
{
    return 0;
};

int CMHICReader::print_ticket(char* chartoprint)
{
    return 0;
};


//IO PORT INTERFACE

unsigned char CMHICReader::ReadPortByteData(unsigned short wChannel)
{
    return 0;
}

unsigned char CMHICReader::WritePortByteData(unsigned short wChannel, unsigned char wData)
{
    return 0;
}

unsigned char CMHICReader::WritePortDwordData(unsigned short wChannel, unsigned int dwData)
{
    return 0;
}

unsigned char CMHICReader::ReadPortDwordData(unsigned short wChannel)
{
    return 0;
}

unsigned char CMHICReader::WriteIOBit(unsigned short wChannel, unsigned short wBit, unsigned short wState)
{
    return 0;
}

extern "C" CBaseDevice* create()
{
    return new CMHICReader;
}

extern "C" void destroy(CBaseDevice* p)
{
    delete p;
}


