#include <iostream>
#include <windows.h>

using std::cout;
using std::cin;
using std::endl;
using std::wcout;

BOOL FileReverse(PCTSTR pszFileName, BOOL& isUnicode)
{
	BOOL nRet = FALSE;
	HANDLE hFile = CreateFile(pszFileName, GENERIC_READ | GENERIC_WRITE, 0, nullptr,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		wcout << "CreateFile: " << pszFileName << "\t Fail" << endl;
		return nRet;
	}
	DWORD dwFileSize = GetFileSize(hFile, nullptr);

	//在文件的最后加上一个\0 由于不知道是ANSI还是UNICODE  所以在后面加上2 确保不会字符串操作越界

	HANDLE hFileMap = CreateFileMapping(hFile, nullptr, PAGE_READWRITE,
		0, dwFileSize + sizeof(wchar_t), nullptr);

	if(!hFileMap)
	{
		CloseHandle(hFile);
		wcout << "CreateFileMapping Failed GLE: " << GetLastError() << endl;
		return nRet;
	}

	LPVOID pvFile = MapViewOfFile(hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);

	if(!pvFile)
	{
		wcout << "MapViewOfFile failed GLE: " << GetLastError() << endl;
		CloseHandle(hFileMap);
		CloseHandle(hFile);
		return nRet;
	}

	int dwRes = 0;
	isUnicode = IsTextUnicode(pvFile, dwFileSize, &dwRes);

	if(!isUnicode)
	{
		PSTR pchANSI = static_cast<PSTR>(pvFile);
		pchANSI[dwFileSize] = '\0';

		_strrev(pchANSI);

		pchANSI = strstr(pchANSI, "\n\r");
		while(pchANSI)
		{
			*pchANSI++ = '\r';
			*pchANSI++ = '\n';
			pchANSI = strstr(pchANSI, "\n\r");
		}
	}
	else
	{
		PWSTR pchUnicode = static_cast<PWSTR>(pvFile);

		if (dwRes == IS_TEXT_UNICODE_SIGNATURE)
			pchUnicode++;

		_wcsrev(pchUnicode);

		pchUnicode = wcsstr(pchUnicode, L"\n\r");
		while (pchUnicode)
		{
			*pchUnicode++ = L'\r';
			*pchUnicode++ = L'\n';
			pchUnicode = wcsstr(pchUnicode, L"\n\r");
		}
	}
	UnmapViewOfFile(pvFile);
	CloseHandle(hFileMap);
	CloseHandle(hFile);
	nRet = TRUE;
	return nRet;
}
BOOL ReadHugeFile(PCTSTR pszFileName)
{
	BOOL nRet = FALSE;

	HANDLE hFile = CreateFile(pszFileName, GENERIC_READ | GENERIC_WRITE, 0, nullptr,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		wcout << "CreateFile: " << pszFileName << "\t Fail" << endl;
		return nRet;
	}

	int64_t dwFileSize;
	LARGE_INTEGER li;

	GetFileSizeEx(hFile, &li);
	dwFileSize = li.QuadPart;

	//对整个文件进行映射，然后在根据偏移 每次读取分配粒度大小的内存的视图，while循环 直到读取结束
	HANDLE hFileMap = CreateFileMapping(hFile, nullptr, PAGE_READWRITE,
		0, 0, nullptr);

	if (!hFileMap)
	{
		CloseHandle(hFile);
		wcout << "CreateFileMapping Failed GLE: " << GetLastError() << endl;
		return nRet;
	}

	int64_t curFileSize = dwFileSize;
	int64_t curPointSet = 0;
	SYSTEM_INFO si = {};
	GetSystemInfo(&si);
	DWORD dwGranularity = si.dwAllocationGranularity;

	//
	while(curFileSize > 0)
	{
		DWORD dwBlockSize = curFileSize > dwGranularity ? dwGranularity : curFileSize;
		LPVOID pvFile = MapViewOfFile(hFileMap, FILE_MAP_READ, static_cast<DWORD>(curPointSet >> 32),
			static_cast<DWORD>(curPointSet & 0xFFFFFFFF), dwBlockSize);

		curFileSize -= dwBlockSize;
		curPointSet += dwBlockSize;
	}

}
int main(int argc, char* argv[])
{
	int64_t in;
	int32_t a;
	int16_t b;
	std::wcout.imbue(std::locale("chs"));

	BOOL isUnicodeFile = FALSE;
	PCTSTR pszName = LR"(D:\zhaoshuhao\test.txt)";
	if (FileReverse(pszName, isUnicodeFile))
		wcout << "reverse File success" << endl;
	else
		wcout << "reverse File Failed" << endl;
}
