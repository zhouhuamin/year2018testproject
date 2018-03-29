#ifndef _SIMPLE_CONFIG_H_
#define	_SIMPLE_CONFIG_H_

#pragma warning(disable : 4786)	
#include <string>
#include <vector>
#include <map>
using namespace std;

#include "tinyXML/xml_config.h"


class CSimpleConfig
{
public:
    CSimpleConfig();

    virtual ~CSimpleConfig();

    void get_config();
    static int GetProcessPath(char* cpPath);

 
    static char EXE_FULL_PATH[256];
    static int RECO_LISTEN_PORT;

};

#endif

