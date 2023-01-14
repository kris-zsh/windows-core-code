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
	if (lpOverlapped->Internal == STATUS_PENDING)
	{
		if (bWait)
		{
			HANDLE hHandle = lpOverlapped->hEvent == nullptr ? hFile : lpOverlapped->hEvent;
			dwRet = WaitForSingleObject(hHandle, INFINITE);
		}
		if (dwRet == WAIT_TIMEOUT)
		{
			SetLastError(ERROR_IO_INCOMPLETE);
			return (FALSE);
		}
		if (dwRet != WAIT_OBJECT_0)
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

#define BUFFSIZE (64 * 1024)
#define MAX_COUNT 4

enum CompeletKey
{
	CW_READ = 0,
	CW_WRITE
};

using PENSURECLEANUP = void (WINAPI *)(UINT_PTR);

template <class TYPE, PENSURECLEANUP pfn, UINT_PTR tInvaild = 0>
class EnsureCleanUp
{
	UINT_PTR _mt;

public:
	EnsureCleanUp() { _mt = tInvaild; }

	EnsureCleanUp(TYPE t) : _mt(UINT_PTR(t))
	{
	}

	~EnsureCleanUp() { CleanUp(); }

	BOOL isValid() { return _mt != tInvaild; }
	BOOL isInvalid() { return !isValid(); }

	void CleanUp()
	{
		if (isValid())
		{
			pfn(_mt);
			_mt = tInvaild;
		}
	}

	TYPE operator =(TYPE t)
	{
		CleanUp();
		_mt = static_cast<UINT_PTR>(t);
		return (*this);
	}

	operator TYPE()
	{
		return TYPE(_mt);
	}
};

#define MakeCleanupClassX(className, pfnCleanUp, dataType, tInvaild)\
	using className = EnsureCleanUp<dataType, (PENSURECLEANUP)pfnCleanUp, (UINT_PTR)tInvaild>


MakeCleanupClassX(FileCleanUp, CloseHandle, HANDLE, INVALID_HANDLE_VALUE);


class CIOReq : public OVERLAPPED
{
	size_t m_BuffSize;
	PVOID p_data;

public:
	CIOReq()
	{
		Offset = 0;
		OffsetHigh = 0;
		hEvent = nullptr;
		Internal = 0;
		InternalHigh = 0;
		m_BuffSize = 0;
		p_data = nullptr;
	}

	~CIOReq()
	{
		VirtualFree(p_data, 0, MEM_RELEASE);
	}

	BOOL AllocSpace(size_t Buffsize)
	{
		m_BuffSize = Buffsize;
		p_data = VirtualAlloc(nullptr, m_BuffSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		return p_data != nullptr;
	}

	BOOL Read(HANDLE hDevice, PLARGE_INTEGER li = nullptr)
	{
		if (li)
		{
			Offset = li->LowPart;
			OffsetHigh = li->HighPart;
		}

		return (ReadFile(hDevice, p_data, m_BuffSize, nullptr, this));
	}

	BOOL Write(HANDLE hDevice, PLARGE_INTEGER li = nullptr)
	{
		if (li)
		{
			Offset = li->LowPart;
			OffsetHigh = li->HighPart;
		}

		return WriteFile(hDevice, p_data, m_BuffSize, nullptr, this);
	}
};

class CIOCP
{
	HANDLE hIOCP;
public:

	CIOCP(int maxConcurrentCnt = -1)
	{
		hIOCP = nullptr;
		if (maxConcurrentCnt != -1)
			Create(maxConcurrentCnt);
	}
	~CIOCP()
	{
		if (hIOCP)
			CloseHandle(hIOCP);
	}

	void Create(int max_concurrent_cnt)
	{
		hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, max_concurrent_cnt);
	}

	void AssociateDevice(HANDLE hDevice, ULONG_PTR uKey)
	{
		CreateIoCompletionPort(hDevice, hIOCP, uKey, 0);
	}
	
	BOOL GetStatus(LPDWORD lpNumberOfBytesTransferred, PULONG_PTR lpCompletionKey, 
		LPOVERLAPPED* lpOverlapped, DWORD dwMilliseconds = INFINITE)
	{
		return GetQueuedCompletionStatus(hIOCP, lpNumberOfBytesTransferred, lpCompletionKey,
			lpOverlapped, dwMilliseconds);
	}

	BOOL PostStatus(DWORD dwNumberOfBytesTransferred, 
		ULONG_PTR dwCompletionKey = 0, LPOVERLAPPED lpOverlapped = nullptr)
	{
		return PostQueuedCompletionStatus(hIOCP, dwNumberOfBytesTransferred, dwCompletionKey, lpOverlapped);
	}
};

template <class MT, class MV>
MT RoundDown(MT szval, MV multiply)
{
	MT temp = (szval / multiply) * multiply;
	temp += (szval % multiply) ? multiply : 0;
	return temp;
}

BOOL MyFileCopy(PCTSTR pszSrc, PCTSTR pszDest)
{
	LARGE_INTEGER liSrc = {}, liDest = {};
	BOOL Ok = FALSE;
	DWORD loopCnt = 0;

	try
	{
		{
			FileCleanUp hSrc = CreateFile(pszSrc, GENERIC_READ, FILE_SHARE_READ, nullptr,
				OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED, nullptr);
			if (hSrc.isInvalid()) goto leave;

			GetFileSizeEx(hSrc, &liSrc);
			liDest.QuadPart = RoundDown(liSrc.QuadPart, BUFFSIZE);

			FileCleanUp hDest = CreateFile(pszDest, GENERIC_WRITE, 0, nullptr,
				CREATE_ALWAYS, FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED,hSrc);
			if (hDest.isInvalid()) goto leave;

			SetFilePointerEx(hDest, liDest, nullptr, FILE_BEGIN);
			SetEndOfFile(hDest);

			CIOCP iocp(0);
			iocp.AssociateDevice(hDest, CompeletKey::CW_WRITE);
			iocp.AssociateDevice(hSrc, CompeletKey::CW_READ);

			DWORD dwReadInProcess = {}, dwWriteInProcess = {};
			LARGE_INTEGER liCur = {};

			CIOReq IOR[MAX_COUNT] = {};for(int i = 0; i < _countof(IOR); i++)
			{
				IOR[i].AllocSpace(BUFFSIZE);
				dwWriteInProcess++;
				iocp.PostStatus(0, CompeletKey::CW_WRITE, &IOR[i]);
			}
			while( dwReadInProcess > 0 | dwWriteInProcess > 0)
			{
				BOOL bResult = FALSE;
				CIOReq* IOR = {};
				DWORD dwByte = {};
				ULONG_PTR CompleteKey = {};


				bResult = iocp.GetStatus(&dwByte, &CompleteKey, (LPOVERLAPPED*)&IOR);
				switch (CompleteKey)
				{
				case CompeletKey::CW_WRITE:
					dwWriteInProcess--;
					if(liCur.QuadPart < liDest.QuadPart)
					{
						dwReadInProcess++;
						bResult = IOR->Read(hSrc, &liCur);
						liCur.QuadPart += BUFFSIZE;
					}
					break;
				case CompeletKey::CW_READ:
					dwWriteInProcess++;
					dwReadInProcess--;
					bResult = IOR->Write(hDest);
					break;
				}
				loopCnt++;
			}
			Ok = TRUE;
		}
	leave:;
	}catch (...)
	{
		
	}
	FileCleanUp hFile = CreateFile(pszDest, GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
		0, nullptr);
	if(hFile.isValid())
	{
		SetFilePointerEx(hFile, liSrc, nullptr, FILE_BEGIN);
		SetEndOfFile(hFile);
	}
	cout << "while loopcnt: " << loopCnt << endl;
	return TRUE;
}
int main()
{
	std::wcout.imbue(std::locale("chs"));

	// FileCleanUp hFile = CreateFileA("test.zsh", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS,
	// 	FILE_FLAG_OVERLAPPED, nullptr);
	// if (hFile == INVALID_HANDLE_VALUE)
	// {
	// 	cout << "GLE: " << GetLastError() << endl;
	// 	return 0;
	//
	// }
	MyFileCopy(LR"(D:\迅雷下载\[www.domp4.cc]地x引l.2013.BD1080p.国英双语.中英双字 (1).mp4)",
		LR"(D:\迅雷下载\[www.domp4.cc]地x引l.2013.BD1080p.国英双语.中英双字 (1).mp4_1)");
	return 0;
}
