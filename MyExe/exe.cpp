#include<windows.h>
#include <iostream>

#include "MyExe.h"

using namespace std;

using PADD = int (*)(int, int);

DWORD WINAPI func(LPVOID szParam)
{
	HMODULE hDll = (HMODULE)szParam;

	PADD padd = (PADD)GetProcAddress(hDll, "abb");
	if (!padd)
	{
		cout << "GetProcAddress failed GLE: " << GetLastError() << endl;
		return 0;
	}
	cout << padd(1, 2) << endl;

}

int main(int argc, char* argv[])
{
	HMODULE hDll = LoadLibraryA("19-basedll.dll");
	if(!hDll)
	{
		cout << "init DLl failed GLE: " << GetLastError() << endl;
		return 0;
	}
	// RebaseImage
	HANDLE hThread = CreateThread(nullptr, 0, func, hDll, 0, nullptr);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	FreeLibrary(hDll);
}
