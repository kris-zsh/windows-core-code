#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <psapi.h>

#pragma comment(lib,"Psapi.lib")

using namespace std;

int main(int argc, char* argv[])
{

	MEMORYSTATUS ms = {sizeof(ms)};
	GlobalMemoryStatus(&ms);
	wcout << L"dwAvailPageFile: "<< ms.dwAvailPageFile << endl;
	wcout << L"dwAvailPhys: " << ms.dwAvailPhys << endl;
	wcout << L"dwAvailVirtual: "<< ms.dwAvailVirtual << endl;
	wcout << L"dwMemoryLoad: " << ms.dwMemoryLoad << endl;
	wcout << L"dwTotalPageFile: "<< ms.dwTotalPageFile << endl;
	wcout << L"dwTotalVirtual: "<< ms.dwTotalVirtual << endl;

	PROCESS_MEMORY_COUNTERS_EX pmc = {sizeof(pmc)};
	GetProcessMemoryInfo(GetCurrentProcess(), (PPROCESS_MEMORY_COUNTERS)&pmc, sizeof(pmc));
	wcout << L"WorkingSetSize: "<< pmc.WorkingSetSize << endl;
	wcout << L"PrivateUsage: " << pmc.PrivateUsage << endl;
}
