#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <chrono>

using namespace std;

void ReadUniCodeFile(HANDLE hFile)
{
	DWORD dwSize = GetFileSize(hFile, nullptr);

	auto szBuff = new wchar_t[dwSize + 1]{};
	wchar_t* temp = szBuff;
	DWORD dwReadByte;
	DWORD ReadByte = dwSize;

	ReadFile(hFile, szBuff, ReadByte, &dwReadByte, nullptr);

	wcout << "dwReadByte: " << dwReadByte << "ReadByte: " << ReadByte << endl;

	int len = WideCharToMultiByte(CP_ACP, 0, szBuff, -1, nullptr, 0, nullptr, nullptr);

	cout << "len: " << len << endl;
	auto szBuf = new char[len]{};

	WideCharToMultiByte(CP_ACP, 0, szBuff, -1, szBuf, len, nullptr, nullptr);
	cout << szBuf << endl;

	delete[] szBuff;
	delete[] szBuf;
}

void ReadANSIFile(HANDLE hFile)
{
	DWORD dwSize = GetFileSize(hFile, nullptr);
	auto szBuff = new char[dwSize + 1]{};

	DWORD dwReaded;
	ReadFile(hFile, szBuff, dwSize, &dwReaded, nullptr);
	cout << szBuff;
	delete[] szBuff;
}

void WriteUnicodeFile(HANDLE hFile, wchar_t* szBuf)
{
	const wchar_t* temp = szBuf;

	DWORD dwWriteData = _tcslen(szBuf);
	DWORD dwWriteToData = 0;

	WriteFile(hFile, temp, dwWriteData, &dwWriteToData, nullptr);
}

void WriteANSIFile(HANDLE hFile, char* szBuf)
{
	const char* temp = szBuf;

	DWORD dwWriteData = strlen(szBuf);
	DWORD dwWriteToData = 0;

	WriteFile(hFile, temp, dwWriteData, &dwWriteToData, nullptr);
}

void testReadWriteFile()
{
	auto pszName = R"(test.zsh)";

	HANDLE hFile = CreateFileA(pszName, GENERIC_READ | GENERIC_WRITE,
	                           FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		wcout << L"GLE: " << GetLastError() << endl;
		return;
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

	if (!(Att & FILE_ATTRIBUTE_HIDDEN))
	{
		SetFileAttributesA(pszName, Att | FILE_ATTRIBUTE_HIDDEN);
	}

	char szBuf[] = "树昊辛苦了|123213123\n";

	DWORD a = 0xfeff;

	for (int i = 0; i < 10; i++)
		WriteANSIFile(hFile, szBuf);

	DWORD re = SetFilePointer(hFile, 0, nullptr, FILE_BEGIN);
	if (re == INVALID_SET_FILE_POINTER)
	{
		wcout << GetLastError() << endl;
	}


	ReadANSIFile(hFile);

	CloseHandle(hFile);
}

void AsynDeviceIO()
{
	HANDLE hFile = CreateFileA("test.zsh", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS,
	                           FILE_FLAG_OVERLAPPED, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		cout << "GLE: " << GetLastError() << endl;
		return;
	}

	DWORD dwFileSize = GetFileSize(hFile, nullptr);

	OVERLAPPED ov = {};
	auto buff = new BYTE[10 + 1]{};
	DWORD i = {};
	cout << i << endl;
	ov.Offset = 0;
	BOOL bRet = ReadFile(hFile, buff, 10, nullptr, &ov);
	if (bRet == FALSE && GetLastError() == ERROR_IO_PENDING)
	{
		cout << "this operator is asynchronously, wait" << endl;
		WaitForSingleObject(hFile, INFINITE);
	}
	cout << "Internal: " << ov.Internal << endl;
	cout << "InternalHigh: " << ov.InternalHigh << endl;
	cout << "ReadData : " << buff << endl;
}

HANDLE hHandles[2] = {};

DWORD WINAPI WaiteAsynComplete(PVOID pParam)
{
	while (true)
	{
		DWORD nRet = WaitForMultipleObjects(2, hHandles, FALSE, INFINITE);
		switch (nRet - WAIT_OBJECT_0)
		{
		case 0:
			cout << "WriteFile success" << endl;
			break;
		case 1:
			cout << "ReadFile success" << endl;
			break;
		default:
			cout << "GLE: " << GetLastError() << endl;
		}
	}
}

void AsynEventIO()
{
	HANDLE hFile = CreateFileA("test.zsh", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS,
	                           FILE_FLAG_OVERLAPPED, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		cout << "GLE: " << GetLastError() << endl;
		return;
	}

	DWORD dwFileSize = GetFileSize(hFile, nullptr);
	hHandles[0] = CreateEventA(nullptr, FALSE, FALSE, nullptr);
	hHandles[1] = CreateEventA(nullptr, FALSE, FALSE, nullptr);

	BYTE f[10] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
	OVERLAPPED ov = {};
	ov.Offset = dwFileSize;
	ov.hEvent = hHandles[0];
	WriteFile(hFile, f, sizeof(f), nullptr, &ov);

	OVERLAPPED ov1 = {};
	ov1.Offset = 0;
	ov1.hEvent = hHandles[1];
	BYTE ff[11] = {};

	ReadFile(hFile, ff, sizeof(f) - 1, nullptr, &ov1);

	CreateThread(nullptr, 0, WaiteAsynComplete, nullptr, 0, nullptr);

	//假装在做什么事情
	std::this_thread::sleep_for(std::chrono::seconds(10));
	cout << "ReadData: " << ff << endl;
}

BOOL
WINAPI
MyGetOverlappedResult(
	_In_ HANDLE hFile,
	_In_ LPOVERLAPPED lpOverlapped,
	_Out_ LPDWORD lpNumberOfBytesTransferred,
	_In_ BOOL bWait
)
{
	
	DWORD dwRet = WAIT_TIMEOUT;
	if(lpOverlapped->Internal == STATUS_PENDING)
	{
		if(bWait)
		{
			HANDLE hHandle = lpOverlapped->hEvent == nullptr ? hFile : lpOverlapped->hEvent;
			dwRet = WaitForSingleObject(hHandle, INFINITE);
		}
		if(dwRet == WAIT_TIMEOUT)
		{
			SetLastError(ERROR_IO_INCOMPLETE);
			return (FALSE);
		}
		if(dwRet != WAIT_OBJECT_0)
		{
			return (FALSE);
		}
		*lpNumberOfBytesTransferred = lpOverlapped->InternalHigh;

		if (SUCCEEDED(lpOverlapped->Internal))
			return (TRUE);

		SetLastError(lpOverlapped->Internal);
		return (FALSE);
	}
}

int main()
{
	std::wcout.imbue(std::locale("chs"));

	return 0;
}
