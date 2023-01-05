#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <TlHelp32.h>

using namespace std;

DWORD WINAPI func(PVOID szParam)
{
	HANDLE hParent = (HANDLE*)szParam;
	FILETIME CreationTime, ExitTime, KernelTime, UserTime;

	GetThreadTimes(hParent, &CreationTime, &ExitTime, &KernelTime, &UserTime);
	CloseHandle(hParent);

	return 0;
}
void MySuspendThread(DWORD dwProcessId, BOOL isSuspend)
{
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	THREADENTRY32 te = { sizeof(te) };
	BOOL re = Thread32First(hSnapShot, &te);

	for (; re; re = Thread32Next(hSnapShot, &te))
	{
		if (te.th32OwnerProcessID == dwProcessId)
		{
			HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
			if (!hThread)
			{
				wcout << "OpenThreadId: " << te.th32ThreadID << " fail GEL: " << GetLastError() << endl;
				continue;
			}
			if (isSuspend)
				SuspendThread(hThread);
			else
				ResumeThread(hThread);
			wcout << "ThreadID: " << te.th32ThreadID << endl;
			CloseHandle(hThread);
		}
	}
	CloseHandle(hSnapShot);
}
int main(int argc, char* argv[])
{
	std::wcout.imbue(std::locale("chs"));

	MySuspendThread(42260, FALSE);
}
