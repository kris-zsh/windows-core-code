#include<Windows.h>
#include<tchar.h>
#include<Tlhelp32.h>
#include <iostream>
#include <shlobj_core.h>


//OpenProcessToken
//GetTokenInformation TokenElevationType
//type == TokenElevationTypeLimited
//	CreateWellKnownSid SECURITY_MAX_SID_SIZE
//	GetTokenInformation TokenLinkedToken
//	CheckTokenMembership
//type == TokenElevationTypeFull
//	isUserAnAdmin

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


int main()
{
	TOKEN_ELEVATION_TYPE t1;
	BOOL isAdmin = FALSE;
	BOOL re = GetProcessElevation(t1, isAdmin);
	printf("TOKEN_ELEVATION_TYPE %d\n", t1);
	if (re)
	{
		if (isAdmin)
			printf("is admin account");
		else
			printf("is normal account");
	}
}