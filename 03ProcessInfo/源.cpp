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
	// ��õ�ǰ���̵����ƾ����
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

		// ����һ������ԱSID
		bRet = CreateWellKnownSid(WinBuiltinAdministratorsSid, nullptr, &byAdmin, &dwsize);
		if (!bRet)
		{
			printf("CreateWellKnownSid failed GLE:[%d]\n", GetLastError());
			return FALSE;
		}
		HANDLE hFilter;
		dwsize = 0;
		//ͨ��Filter Token�����ԭʼ��Token
		//TokenLinkedToken��־����ʾҪ���Filter Token��ԭʼToken��
		GetTokenInformation(hToken, TokenLinkedToken, &hFilter, sizeof(hFilter), &dwsize);
		// ���ԭʼToken�У�����Ա�˻�adminSID�Ƿ񱻼������������ô˵���������
		//������ʺ��ǹ���Ա�ʺţ�������
		//���CheckTokenMembership�����������������pIsAdmin�����У�����������ķ���ֵ
		//ֻ�Ǳ�ʾ����������Ƿ�ɹ���
		CheckTokenMembership(hFilter, &byAdmin, &isAdmin);
		printf(" TOKEN �Ǻ��� ԭʼtoken %s \n", isAdmin == 1 ? "admin" : "default");
		CloseHandle(hFilter);
		CloseHandle(hToken);
		return TRUE;
	}
	//�����ԭʼ���ƣ�ֻҪIsUsrAndmin�Ϳ���ȷ����������ǰ������ʺ��Ƿ��ǹ���Ա�ʺš�
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