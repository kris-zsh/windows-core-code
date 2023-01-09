#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <strsafe.h>
using namespace std;


void GetCurrentProcessIsWow64()
{
	wchar_t szBuff[1024];

#ifdef _WIN64
	StringCchPrintf(szBuff, _countof(szBuff), L"CurrentProcess is 64-bit");
#else

	BOOL isWow64;
	if(IsWow64Process(GetCurrentProcess(), &isWow64))
	{
		if (isWow64)
		{
			StringCchPrintf(szBuff, _countof(szBuff), L"CurrentProcess is 32bit App run on Wow64");
		}
		else
		{
			StringCchPrintf(szBuff, _countof(szBuff), L"CurrentProcess is 32bit App on 32-bit system");
		}
	}
	else
		StringCchPrintf(szBuff, _countof(szBuff), L"Unknown");

#endif
	wcout << szBuff << endl;
}

void MyGetSystemInfo()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	wcout << L"page size : " << si.dwPageSize <<endl;
	wcout << L"Minimum Application Address: " << si.lpMinimumApplicationAddress << endl;
	wcout << L"Maximum Application Address: " << si.lpMaximumApplicationAddress << endl;
	wcout << L"Number Of Processors : " << si.dwNumberOfProcessors << endl;
	wcout << L"Allocation Granularity : " << si.dwAllocationGranularity << endl;

	if(si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
		wcout << L"Processor Architecture Intel or Amd" << endl;
	else
		wcout << L"Processor Architecture Unknown" << endl;
}
int main(int argc, char* argv[])
{
	GetCurrentProcessIsWow64();
	MyGetSystemInfo();
	return 0;
	
}
