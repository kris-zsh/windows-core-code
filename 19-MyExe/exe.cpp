#include<windows.h>
#include <iostream>

#include "MyExe.h"

using namespace std;

using PADD = int (__stdcall *)(int, int);


DWORD WINAPI func(LPVOID szParam)
{
	HMODULE hDll = (HMODULE)szParam;

	PADD padd = (PADD)GetProcAddress(hDll, "abb");
	if (!padd)
	{
		std::cout << "GetProcAddress failed GLE: " << GetLastError() << endl;
		return 0;
	}
	std::cout << padd(1, 2) << endl;
	return 0;
}

int main(int argc, char* argv[])
{
	HMODULE hDll = LoadLibraryA("19-basedll.dll");
	if(!hDll)
	{
		cout << "init DLl failed GLE: " << GetLastError() << endl;
		return 0;
	}

	HANDLE hThread = CreateThread(nullptr, 0, func, hDll, 0, nullptr);
	WaitForSingleObject(hThread, INFINITE);


	CloseHandle(hThread);
	FreeLibrary(hDll);
}
