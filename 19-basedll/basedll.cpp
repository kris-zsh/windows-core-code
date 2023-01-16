#include <windows.h>
#include <iostream>
#include "basedll.h"

using namespace std;

int __stdcall add(int a, int b)
{
	return a + b;
}

// class my_class
// {
// public:
// 	my_class()
// 	{
// 		cout << "my_class construct" << endl;
// 	}
// }GLOBAL_A;

// DWORD WINAPI fun(PVOID)
// {
// 	cout << "this is dll thread" << endl;
// 	return 0;
// }
BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD fdwReason, PVOID fImpLoad)
{
	
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		//在赋值的时候需要加上定义
		cout << "DLL_process_attach" << endl;
		{// HANDLE hThread = CreateThread(nullptr, 0, fun, nullptr, 0, nullptr);
		// WaitForSingleObject(hThread, INFINITE);
		}
		break;
	case DLL_PROCESS_DETACH:
		cout << "DLL_PROCESS DETACH" << endl;
		break;
	case DLL_THREAD_ATTACH:
		cout << "DLL thread attach" << endl;
		break;
	case DLL_THREAD_DETACH:
		cout << "dll thread detach" << endl;
		break;
	}
	return TRUE;
}
