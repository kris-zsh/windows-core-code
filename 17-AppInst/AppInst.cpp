#include <windows.h>
#include <iostream>
#include <string>
#include <tchar.h>

using namespace std;

#pragma data_seg("shared")
volatile DWORD dwInstCount = 0;
#pragma data_seg()

#pragma comment(linker, "/SECTION:shared,RWS")
// #pragma comment(linker,"/SECTION:Shared,RWS")

int main(int argc, char* argv[])
{
	InterlockedExchangeAdd(&dwInstCount, 1);

	MessageBoxA(nullptr, to_string(dwInstCount).c_str(), nullptr, 0);

	InterlockedExchangeAdd(&dwInstCount, -1);
}
