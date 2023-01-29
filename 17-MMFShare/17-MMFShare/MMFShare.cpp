#include <windows.h>
#include <iostream>

using std::wcout;
using std::endl;


int main(int argc, char* argv[])
{
	enum { MAX_BUFF = 4 * 1024 };
	HANDLE hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
	                                    0, MAX_BUFF, L"ZSHShare");
	if(GetLastError() == ERROR_ALREADY_EXISTS)
	{
		wchar_t* lpFile = (wchar_t*)MapViewOfFile(hMapFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, MAX_BUFF);

		wcout << lpFile << endl;
		system("pause");
		UnmapViewOfFile(lpFile);
	}
	else
	{
		wchar_t* lpFile = (wchar_t*)MapViewOfFile(hMapFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, MAX_BUFF);

		const wchar_t szBuff[] = L"shuhao xinkule";
		wcscpy_s(lpFile, sizeof(szBuff) / sizeof(wchar_t), szBuff);
		system("pause");
		UnmapViewOfFile(lpFile);
	}

	CloseHandle(hMapFile);
}
