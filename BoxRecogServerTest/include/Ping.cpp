#include "StdAfx.h"
#include "Ping.h"

CPing::CPing()
{
	bValid = FALSE;
	WSADATA wsaData;			// WSADATA
	int nRet;					// General use return code

	// Dynamically load the ICMP.DLL
	hndlIcmp = ::LoadLibraryA("ICMP.DLL");
	if (hndlIcmp == NULL)
	{
		::MessageBox(NULL, _T("Could not load ICMP.DLL"), _T("Error:"), MB_OK);
		return;
	}
	// Retrieve ICMP function pointers
	pIcmpCreateFile  = (HANDLE (WINAPI *)(void))
		GetProcAddress((HMODULE)hndlIcmp,"IcmpCreateFile");
	pIcmpCloseHandle = (BOOL (WINAPI *)(HANDLE))
		GetProcAddress((HMODULE)hndlIcmp,"IcmpCloseHandle");
	pIcmpSendEcho = (DWORD (WINAPI *)
		(HANDLE,DWORD,LPVOID,WORD,PIPINFO,LPVOID,DWORD,DWORD))
		GetProcAddress((HMODULE)hndlIcmp,"IcmpSendEcho");
	// Check all the function pointers
	if (pIcmpCreateFile == NULL		|| 
		pIcmpCloseHandle == NULL	||
		pIcmpSendEcho == NULL)
	{
		//::MessageBox(NULL, "Error loading ICMP.DLL", "Error:", MB_OK);
		FreeLibrary((HMODULE)hndlIcmp);
		return;
	}

	// Init WinSock
	nRet = WSAStartup(0x0101, &wsaData );
    if (nRet)
    {
		::MessageBox(NULL, _T("WSAStartup() error:"), _T("Error:"), MB_OK);
        WSACleanup();
		FreeLibrary((HMODULE)hndlIcmp);
        return;
    }
    // Check WinSock version
    if (0x0101 != wsaData.wVersion)
    {
		::MessageBox(NULL, _T("No WinSock version 1.1 support found"), _T("Error:"), MB_OK);
        WSACleanup();
		FreeLibrary((HMODULE)hndlIcmp);
        return;
    }
	bValid = TRUE;
}

CPing::~CPing()
{
    WSACleanup();
	FreeLibrary((HMODULE)hndlIcmp);
}

int CPing::Ping(char* strHost)
{
	struct in_addr iaDest;		// Internet address structure
    LPHOSTENT pHost;			// Pointer to host entry structure
	DWORD *dwAddress;			// IP Address
	IPINFO ipInfo;				// IP Options structure
	ICMPECHO icmpEcho;			// ICMP Echo reply buffer
	HANDLE hndlFile;			// Handle for IcmpCreateFile()

    if(!bValid)
	{
		return FALSE;
	}

	// Lookup destination
    // Use inet_addr() to determine if we're dealing with a name
    // or an address

    int   nRemoteAddr  = inet_addr(strHost);
    //if (iaDest.s_addr == INADDR_NONE)
    //    pHost = gethostbyname(strHost);
    //else
     pHost =gethostbyname(strHost);
	if (pHost == NULL)
	{
		return FALSE;
	}

	// Copy the IP address
	dwAddress = (DWORD *)(*pHost->h_addr_list);

	// Get an ICMP echo request handle        
	hndlFile = pIcmpCreateFile();

	// Set some reasonable default values
	ipInfo.Ttl = 255;
	ipInfo.Tos = 0;
	ipInfo.IPFlags = 0;
	ipInfo.OptSize = 0;
	ipInfo.Options = NULL;
	icmpEcho.Status = 0;
	// Reqest an ICMP echo
	pIcmpSendEcho(
		hndlFile,		// Handle from IcmpCreateFile()
		*dwAddress,		// Destination IP address
		NULL,			// Pointer to buffer to send
		0,				// Size of buffer in bytes
		&ipInfo,		// Request options
		&icmpEcho,		// Reply buffer
		sizeof(struct tagICMPECHO),
		1000);			// Time to wait in milliseconds
	// Print the results
	iaDest.s_addr = icmpEcho.Source;
	if (icmpEcho.Status)
	{
		pIcmpCloseHandle(hndlFile);
		return FALSE;
	}
                             
	// Close the echo request file handle
	pIcmpCloseHandle(hndlFile);
	return TRUE;
}
