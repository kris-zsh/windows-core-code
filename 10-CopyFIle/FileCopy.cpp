#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

using namespace std;

void ReadUniCodeFile(HANDLE hFile)
{
	DWORD dwSize = GetFileSize(hFile, nullptr);

	wchar_t* szBuff = new wchar_t[dwSize + 1]{};
	wchar_t* temp = szBuff;
	DWORD dwReadByte;
	DWORD ReadByte = dwSize;

	ReadFile(hFile, szBuff, ReadByte, &dwReadByte, nullptr);

	wcout << "dwReadByte: " <<dwReadByte << "ReadByte: " << ReadByte<< endl;

	int len = WideCharToMultiByte(CP_ACP, 0, szBuff, -1, nullptr, 0, nullptr, nullptr);

	cout << "len: " << len <<endl;
	char* szBuf = new char[len]{};

	WideCharToMultiByte(CP_ACP, 0, szBuff, -1, szBuf, len, nullptr, nullptr);
	cout << szBuf << endl;

	delete[] szBuff;
	delete[] szBuf;
}

void ReadANSIFile(HANDLE hFile)
{
	DWORD dwSize = GetFileSize(hFile, nullptr);
	char* szBuff = new char[dwSize + 1]{};

	DWORD dwReaded;
	ReadFile(hFile, szBuff, dwSize, &dwReaded, nullptr);
	cout << szBuff;
	delete[] szBuff;
}

void WriteUnicodeFile(HANDLE hFile, wchar_t* szBuf)
{
	const wchar_t* temp = szBuf;

	DWORD dwWriteData  = _tcslen(szBuf);
	DWORD dwWriteToData = 0;

	WriteFile(hFile, temp, dwWriteData, &dwWriteToData, nullptr);

}

void WriteANSIFile(HANDLE hFile, char* szBuf)
{
	const char* temp = szBuf;

	DWORD dwWriteData  = strlen(szBuf);
	DWORD dwWriteToData = 0;

	WriteFile(hFile, temp, dwWriteData, &dwWriteToData, nullptr);
}

int main()
{
	std::wcout.imbue(std::locale("chs"));

	LPCSTR pszName = R"(test.zsh)";

	HANDLE hFile = CreateFileA(pszName,  GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		wcout << L"GLE: " << GetLastError() << endl;
		return 0;
	}

	// SetFilePointer(hFile, 0, 0, FILE_BEGIN);
	// SetEndOfFile(hFile);
	// SetFilePointer(hFile, 1024, 0, FILE_BEGIN);
	// SetEndOfFile(hFile);
	//
	// DWORD dwSize = GetFileSize(hFile, 0);
	// cout << "dwSize: " << dwSize <<endl;
	// getchar();

	DWORD Att = GetFileAttributesA(pszName);
	cout << Att << endl;

	if(!(Att & FILE_ATTRIBUTE_HIDDEN))
	{
		SetFileAttributesA(pszName, Att | FILE_ATTRIBUTE_HIDDEN);
	}

	char szBuf[] = "Ê÷ê»ÐÁ¿àÁË|123213123\n";

	DWORD a = 0xfeff;

	for(int i = 0; i < 10; i++)
		WriteANSIFile(hFile, szBuf);

	DWORD re = SetFilePointer(hFile, 0, nullptr, FILE_BEGIN);
	if(re == INVALID_SET_FILE_POINTER)
	{
		wcout << GetLastError() << endl;
	}


	ReadANSIFile(hFile);

	CloseHandle(hFile);
	return 0;
}