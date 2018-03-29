/* 
 * File:   RTUProtocol.h
 * Author: root
 *
 * Created on 2015年1月31日, 下午4:29
 */
#include <vector>
#include <string>
#include <utility>


#ifndef RTUPROTOCOL_H
#define	RTUPROTOCOL_H
typedef unsigned char	BYTE;
typedef unsigned short	WORD;
typedef unsigned int	UINT;
typedef unsigned long	DWORD;
typedef long			LONG;
typedef unsigned short uint16_t;
typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;

#define DWORD_PTR       DWORD


#define MAKEWORD(a, b)      ((WORD)(((BYTE)((DWORD_PTR)(a) & 0xff)) | ((WORD)((BYTE)((DWORD_PTR)(b) & 0xff))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)((DWORD_PTR)(a) & 0xffff)) | ((DWORD)((WORD)((DWORD_PTR)(b) & 0xffff))) << 16))
#define LOWORD(l)           ((WORD)((DWORD_PTR)(l) & 0xffff))
#define HIWORD(l)           ((WORD)((DWORD_PTR)(l) >> 16))
#define LOBYTE(w)           ((BYTE)((DWORD_PTR)(w) & 0xff))
#define HIBYTE(w)           ((BYTE)((DWORD_PTR)(w) >> 8))

struct structRTUDataStruct
{
	BYTE ADDR;			// 0x01
	BYTE CMD;			// 0x53 0x65
	BYTE DATA[252];
	BYTE CRCLow;
	BYTE CRCHigh;
};

#pragma pack(push)
#pragma pack(1)
struct structUploadData
{
	BYTE IDX;
	BYTE Di_Last;
	BYTE Di_Now;
	BYTE Do_Now;
	BYTE JDQ_Now;
	BYTE Error;
};
#pragma pack(pop)

struct structHeartbeatFrame
{
	BYTE ADDR;			// 0x01
	BYTE CMD;			// 0x65
	BYTE DATA;
	BYTE CRCLow;
	BYTE CRCHigh;
};

class RTUProtocol {
public:
    RTUProtocol();
    RTUProtocol(const RTUProtocol& orig);
    virtual ~RTUProtocol();

private:


public:
	unsigned short CalCRC(const std::vector<BYTE> &dataVect);
	void ReadRegisterRequest(std::vector<BYTE> &dataVect, BYTE bySeqNo, BYTE byRegCount);
	bool ReadRegisterResponse(const std::vector<BYTE> &dataVect, BYTE Length);
	void WriteRegisterRequest(std::vector<BYTE> &dataVect);
	bool WriteRegisterResponse(const std::vector<BYTE> &dataVect);
	bool ReadUploadData(const std::vector<BYTE> &dataVectm, std::pair<BYTE, structUploadData> &pairData, const std::string &strReverse);

	uint16 crc16(BYTE *puchMsg, uint16 usDataLen);  
};

#endif	/* RTUPROTOCOL_H */

