#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <tlhelp32.h>
#include <wct.h>

using namespace std;

BOOL EnabledPrivilege(LPCWSTR lpName, BOOL isEnable)
{
	HANDLE hToken;
	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);

	LUID luid;
	LookupPrivilegeValue(nullptr, lpName, &luid);

	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = isEnable ? SE_PRIVILEGE_ENABLED : 0;
	BOOL nRet = AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), nullptr, nullptr);
	if (GetLastError() == ERROR_SUCCESS)
		wcout << "modify privilege success" << endl;
	else
		wcout << "modify privilege failed" << endl;
	CloseHandle(hToken);
	return TRUE;
}

BOOL TestDeadLockChain(DWORD dwPid)
{
	HWCT hWct = OpenThreadWaitChainSession(0, nullptr);

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	THREADENTRY32 te = {sizeof(te)};
	WAITCHAIN_NODE_INFO wni[WCT_MAX_NODE_COUNT];
	BOOL bDeadlock;
	DWORD dwCount = WCT_MAX_NODE_COUNT;
	for (BOOL re = Thread32First(hSnap, &te); re; re = Thread32Next(hSnap, &te))
	{
		if (te.th32OwnerProcessID == dwPid)
		{
			GetThreadWaitChain(hWct, 0, WCTP_GETINFO_ALL_FLAGS, te.th32ThreadID, &dwCount, wni, &bDeadlock);
			wcout <<"ThreadId " << te.th32ThreadID << " is DeadLock enable : " << bDeadlock << endl;
		}
	}
	CloseThreadWaitChainSession(hWct);
	return TRUE;
}

int main(int argc, char* argv[])
{
	EnabledPrivilege(SE_DEBUG_NAME, TRUE);
	TestDeadLockChain(32300);
	EnabledPrivilege(SE_DEBUG_NAME, FALSE);
	system("pause");
}
