/* 
 * File:   BoxNumberCheckAlgo.cpp
 * Author: root
 * 
 * Created on 2015年1月28日, 下午1:56
 */

#include "BoxNumberCheckAlgo.h"

BoxNumberCheckAlgo::BoxNumberCheckAlgo()
{
}

BoxNumberCheckAlgo::BoxNumberCheckAlgo(const BoxNumberCheckAlgo& orig)
{
}

BoxNumberCheckAlgo::~BoxNumberCheckAlgo()
{
}

int BoxNumberCheckAlgo::GetBoxNumCheckbit(const std::string &strBoxNum, char &chVerifyCode)
{
	if (strBoxNum.empty() || strBoxNum.size() != 11)
	{
		chVerifyCode = '\0';
		return 0;
	}

	
	for (int j = 0; j < strBoxNum.size() - 1; ++j)
	{
		if (strBoxNum[j] == '?')
		{
			chVerifyCode = '\0';
			return 0;
		}
	}

	int index = 0;
	int i = 0;
	int nTotal = 0;
	static char szTable[39]={'0','1','2','3','4','5','6','7','8','9',
'A','0','B','C','D','E','F','G','H','I',
'J','K','0','L','M','N','O','P','Q','R',
'S','T','U','0','V','W','X','Y','Z'};

	while (i < 10)
	{
		for(int n = 0; n < 39; ++n)
		{
			if(strBoxNum[i] == szTable[n])
			{
				index = n;
				break;
			}
		}

		nTotal = nTotal + (index << i);
		++i;
	}
	if ((nTotal % 11) == 10)
	{
		chVerifyCode = '0';
		//return 1;
	}
	else
	{
		chVerifyCode = nTotal % 11 + '0';
	}

	if (chVerifyCode == strBoxNum[10])
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


