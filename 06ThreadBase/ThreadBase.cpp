#include <windows.h>
#include <tchar.h>
#include <iostream>

using namespace std;

DWORD WINAPI func(PVOID szParam)
{
	HANDLE hParent = (HANDLE*)szParam;
	FILETIME CreationTime, ExitTime, KernelTime, UserTime;

	GetThreadTimes(hParent, &CreationTime, &ExitTime, &KernelTime, &UserTime);
	CloseHandle(hParent);

	return 0;
}
int main(int argc, char* argv[])
{
	std::wcout.imbue(std::locale("chs"));

	int i = 10;
	HANDLE hProcess = GetCurrentProcess();
	DuplicateHandle(GetCurrentProcess(), GetCurrentProcess(), GetCurrentProcess(), &hProcess, 0, FALSE, DUPLICATE_SAME_ACCESS);

	HANDLE hTread = CreateThread(nullptr, 0, func, hProcess, 0, nullptr);
	WaitForSingleObject(hTread, INFINITE);
}
