#if !defined DATA_PACKET_H
#define DATA_PACKET_H


#include <vector>
#include <map>
#include <list>
#include <string>
using namespace std;

class CDataPacket
{
public:

    CDataPacket()
    {
    }
    ;

    virtual ~CDataPacket()
    {
    };

public:

	virtual int SetHeadData(char* szXML)
    {
        return 0;
    }


    virtual int SetTagData(char* szTag, char* szXML)
    {
        return 0;
    }

    virtual int PacketData(char* szXML)
    {
        return 0;
    }
};

// the types of the class factories
typedef CDataPacket* create_t();
typedef void destroy_t(CDataPacket*);


#endif

