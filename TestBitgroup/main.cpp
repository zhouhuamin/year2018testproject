/* 
 * File:   main.cpp
 * Author: root
 *
 * Created on 2015年11月19日, 下午4:10
 */

#include <cstdlib>
#include <iostream>


typedef unsigned char	BYTE;

#pragma pack(push)
#pragma pack(1)
union TOLEDO_STATUS
{
	struct struSTATUS
	{
		BYTE b0:1;
		BYTE b1:1;
		BYTE b2:1;
		BYTE b3:1;
		BYTE b4:1;
		BYTE b5:1;
		BYTE b6:1;
		BYTE b7:1;
	}status;
	BYTE byteStatus;
};
#pragma pack(pop)


using namespace std;

/*
 * 
 */
int main(int argc, char** argv)
{

    cout << "sizeof is " << sizeof(TOLEDO_STATUS) << endl;
    return 0;
}

