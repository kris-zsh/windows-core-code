#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <sddl.h>
#include <strsafe.h>
// #pragma commit(L"Advapi32.lib")

using namespace std;

const TCHAR* g_boundName = L"zsh";

void CheckInstances()
{
	HANDLE g_hBoundary = CreateBoundaryDescriptor(g_boundName, 0);

	BYTE adminSID[SECURITY_MAX_SID_SIZE];
	DWORD cbSid = sizeof(adminSID);
	BOOL bOpen = FALSE;

	BOOL bRet = FALSE;
	bRet = CreateWellKnownSid(WinBuiltinAdministratorsSid, nullptr, adminSID, &cbSid);
	AddSIDToBoundaryDescriptor(&g_hBoundary, adminSID);

	SECURITY_ATTRIBUTES sa = { sizeof(sa) , nullptr, FALSE};

	ConvertStringSecurityDescriptorToSecurityDescriptor(L"D:(A;;GA;;;BA)", SDDL_REVISION_1, &sa.lpSecurityDescriptor, nullptr);

	HANDLE hNameSpace = CreatePrivateNamespace(&sa, g_hBoundary, g_boundName);
	//hNameSpace == null  this code must run under a Local Administrator account
	if (!hNameSpace)
	{
		hNameSpace = OpenPrivateNamespaceW(&sa, g_boundName);
		bOpen = TRUE;
	}

	LocalFree(sa.lpSecurityDescriptor);

	TCHAR szMut[64];

	StringCchPrintf(szMut, _countof(szMut), L"%s\\%s", g_boundName, L"Mutex");

	HANDLE hMutex = CreateMutex(nullptr, false, szMut);
	if(GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MessageBox(0, L"already exists", 0, 0);
	}
	else
	{
		TCHAR szMess[64];
		StringCchPrintf(szMess, _countof(szMess), L"GetLastError %d", GetLastError());
		MessageBox(0, szMess, 0, 0);
	}

	DeleteBoundaryDescriptor(g_hBoundary);
	if (bOpen)
		ClosePrivateNamespace(hNameSpace, 0);
	else
		ClosePrivateNamespace(hNameSpace, PRIVATE_NAMESPACE_FLAG_DESTROY);
	CloseHandle(hMutex);
}

int WINAPI _tWinMain(HINSTANCE hInstExe, HINSTANCE, PTSTR, int)
{
	
	CheckInstances();
	return 0;
}
