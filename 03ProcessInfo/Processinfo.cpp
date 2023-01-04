#include <algorithm>
#include<Windows.h>
#include<tchar.h>
#include<Tlhelp32.h>
#include <iostream>
#include <shlobj_core.h>
#include <tlhelp32.h>
#include <vector>
#include <winternl.h>


//OpenProcessToken
//GetTokenInformation TokenElevationType
//type == TokenElevationTypeLimited
//	CreateWellKnownSid SECURITY_MAX_SID_SIZE
//	GetTokenInformation TokenLinkedToken
//	CheckTokenMembership
//type == TokenElevationTypeFull
//	isUserAnAdmin
using namespace std;
BOOL GetProcessElevation(TOKEN_ELEVATION_TYPE& ELEVATION_TYPE, BOOL& isAdmin)
{
	HANDLE hToken;
	DWORD dwsize;
	// 获得当前进程的令牌句柄。
	BOOL bRet = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken);
	if (!bRet)
	{
		printf("OpendProcessToken failed GLE:[%d]\n", GetLastError());
		return FALSE;
	}

	bRet = GetTokenInformation(hToken, TokenElevationType, &ELEVATION_TYPE, sizeof(TOKEN_ELEVATION_TYPE), &dwsize);
	if (!bRet)
	{
		printf("GetTokenInformation failed GLE:[%d]\n", GetLastError());
		return FALSE;
	}

	if (ELEVATION_TYPE == TokenElevationTypeLimited)
	{
		BYTE byAdmin[SECURITY_MAX_SID_SIZE];
		dwsize = sizeof(byAdmin);

		// 创建一个管理员SID
		bRet = CreateWellKnownSid(WinBuiltinAdministratorsSid, nullptr, &byAdmin, &dwsize);
		if (!bRet)
		{
			printf("CreateWellKnownSid failed GLE:[%d]\n", GetLastError());
			return FALSE;
		}
		HANDLE hFilter;
		dwsize = 0;
		//通过Filter Token来获得原始的Token
		//TokenLinkedToken标志，表示要获得Filter Token的原始Token。
		GetTokenInformation(hToken, TokenLinkedToken, &hFilter, sizeof(hFilter), &dwsize);
		// 检查原始Token中，管理员账户adminSID是否被激活，如果被激活，那么说明启动这个
		//程序的帐号是管理员帐号，否则不是
		//这个CheckTokenMembership函数，结果被保存在pIsAdmin参数中，而这个函数的返回值
		//只是表示，这个函数是否成功。
		CheckTokenMembership(hFilter, &byAdmin, &isAdmin);
		printf(" TOKEN 是猴版的 原始token %s \n", isAdmin == 1 ? "admin" : "default");
		CloseHandle(hFilter);
		CloseHandle(hToken);
		return TRUE;
	}
	//如果是原始令牌，只要IsUsrAndmin就可以确定，启动当前程序的帐号是否是管理员帐号。
	else if (ELEVATION_TYPE == TokenElevationTypeFull)
		printf("token is Elevation\n");
	else
		printf("token is default\n");
	isAdmin = IsUserAnAdmin();
	CloseHandle(hToken);
	return TRUE;
}
BOOL EnablePrivilege(LPCTSTR pszPrivilege, BOOL fEnable)
{
	HANDLE hToken;
	BOOL nRet = FALSE;
	LUID Luid;

	BOOL bVal = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);
	if (!bVal)
	{
		printf("OpenProcessToken failed %d", GetLastError());
		return nRet;
	}

	LookupPrivilegeValueW(nullptr, pszPrivilege, &Luid);

	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = Luid;

	tp.Privileges[0].Attributes = fEnable ? SE_PRIVILEGE_ENABLED : 0;

	AdjustTokenPrivileges(hToken, FALSE, &tp, NULL, nullptr, nullptr);
	nRet = (GetLastError() == ERROR_SUCCESS);
	CloseHandle(hToken);
	return nRet;
}



DWORD StartElevatedProcess(LPCTSTR pszExcutable, LPCTSTR pszCmdline)
{
	SHELLEXECUTEINFO si = { sizeof(si) };
	si.lpFile = pszExcutable;
	si.lpVerb = TEXT("runas");
	si.lpParameters = pszCmdline;
	si.nShow = SW_SHOWNORMAL;

	ShellExecuteEx(&si);
	return (GetLastError());
}

DWORD FindProcessId(LPCTSTR pszExeName)
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	std::wstring strName = pszExeName;
	PROCESSENTRY32 pe = {sizeof(pe)};
	DWORD dwProcessId = -1;

	BOOL re = Process32First(hSnap, &pe);
	
	for(;re;re = Process32Next(hSnap, &pe))
	{
		std::wcout << pe.szExeFile <<endl;

		int nCmp = strName.compare(pe.szExeFile);
		//传入-1 就会自动帮忙确认长度
		// int nCmp = CompareStringOrdinal(pszExeName, -1, pe.szExeFile, -1,TRUE);
		if(nCmp == 0)
		{
			dwProcessId = pe.th32ProcessID;
			break;
		}
	}
	CloseHandle(hSnap);
	return dwProcessId;
}
DWORD FindThreadId(DWORD& dwProId, vector<DWORD>& dwVec)
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	THREADENTRY32 te = {sizeof(te)};

	BOOL re = Thread32First(hSnap, &te);
	for(; re; re = Thread32Next(hSnap, &te))
	{
		if (dwProId == te.th32OwnerProcessID)
		{
			dwVec.push_back(te.th32ThreadID);
		}
	}
	if (dwVec.empty())
	{
		return FALSE;
	}
	return TRUE;
}

void FindProcessContainThreadId(LPCTSTR pszProcessName)
{
	DWORD dwID = FindProcessId(pszProcessName);
	std::wcout << pszProcessName << " processId: " << dwID << endl;
	if (dwID != -1)
	{
		vector<DWORD> vec;
		BOOL re = FindThreadId(dwID, vec);
		if (re)
		{
			std::wcout << pszProcessName <<" contain threadId" << endl;
			for_each(vec.begin(), vec.end(), [](DWORD a)
			{
				std::wcout << a << endl;
			});
		}
	}
}

using NtQuery = NTSTATUS(*)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);

BOOL GetProcessCmdline(const DWORD& dwProcessID, std::wstring& wstrCmd)
{
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessID);
	if (!hProcess)
		return FALSE;

	HMODULE hDll = LoadLibraryA("Ntdll.dll");
	if (!hDll)
		return FALSE;
	auto Nt = reinterpret_cast<NtQuery>(GetProcAddress(hDll, "NtQueryInformationProcess"));

	PROCESS_BASIC_INFORMATION pbi;
	DWORD dwSize;
	NTSTATUS status = Nt(hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), &dwSize);

	FreeLibrary(hDll);

	if (!NT_SUCCESS(status))
		return FALSE;

	
	PEB peb;
	RTL_USER_PROCESS_PARAMETERS para;
	ReadProcessMemory(hProcess, pbi.PebBaseAddress, &peb, sizeof(peb), nullptr);

	ReadProcessMemory(hProcess, peb.ProcessParameters, &para, sizeof(para), nullptr);

	wchar_t wszCmd[MAX_PATH + 1];
	ReadProcessMemory(hProcess, para.CommandLine.Buffer, wszCmd, MAX_PATH * sizeof(wchar_t), nullptr);
	wstrCmd = std::wstring(wszCmd);

	CloseHandle(hProcess);
	return TRUE;
}
DWORD HowManyHeaps(DWORD dwProcessId)
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST, dwProcessId);

	HEAPLIST32 hl = {sizeof(hl)};
	
	DWORD dwHeapCount = 0;
	for (BOOL re = Heap32ListFirst(hSnap, &hl); re; re = Heap32ListNext(hSnap, &hl))
		dwHeapCount++;

	return dwHeapCount;
}
using IsWow64 = BOOL(WINAPI *)(HANDLE, PBOOL);

BOOL ProcessIsWow64(DWORD dwProcessID)
{
	BOOL bIsWow64 = FALSE;
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, dwProcessID);

	auto isWow = (IsWow64)GetProcAddress(GetModuleHandle(L"Kernel32"), "IsWow64Process");

	if (isWow)
	{
		if (isWow(hProcess, &bIsWow64))
		{
			//Error
		}
	}

	CloseHandle(hProcess);
	return bIsWow64;
}

int main()
{
	std::wcout.imbue(std::locale("chs"));
	std::wcout << ProcessIsWow64(66258) << endl;
	std::wcout << ProcessIsWow64(26200) << endl;
	getchar();
}