/*

typedef struct _MEMORY_BASIC_INFORMATION {
  PVOID  BaseAddress;
  PVOID  AllocationBase;
  DWORD  AllocationProtect;//�����ַ�ڴ�ı�������
  SIZE_T RegionSize;
  DWORD  State;
  DWORD  Protect;//�����ڴ�ı�������
  DWORD  Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;


BaseAddress:	�öεĻ���ַ
AllocationBase��	ʹ��VirtualAlloc�����ڴ�ʱ�����ص��ڴ��ַ��Ҳ���Ƿ����ڴ���׵�ַ
RegionSize��		�δ�С
State��			
				MEM_COMMIT��		�����ڴ��Ѿ������䣬�����ڴ��Ѿ�������	��0x1000��
				MEM_RESERVE��	�����ڴ汻���䣬��û�������ڴ���֮��Ӧ	��0x2000��
				MEM_FREE:		�����ڴ�û�б�����					��0x10000��

Type��			MEM_IMAGE:		һ���ǽ�EXE����DLL�ļ�Ӱ�䵽�ڴ��е��ڴ�����	��0x1000000��
				MEM_MAPPED:		һ�����ڴ�Ӱ���ļ�Ӱ�䵽����ڴ��				��0x40000��
				MEM_PRIVATE:	˽���ڴ�									��0x20000��


AllocationProtect/Protect:	�ڴ�ҳ��ı�������:

	PAGE_EXECUTE:			ֻ��ִ�е��ڴ�Σ���������ж�д��������ô�ͻ����Υ����ʴ��󣬳���ֹͣ���С�
	(0x10)
	PAGE_EXECUTE_READ��		�����Խ���д������������д�����������Υ����ʲ�ֹͣ���С�
	(0x20)
	PAGE_EXECUTE_READWRITE��	���Խ����κβ�����
	(0x40)
	PAGE_EXECUTE_WRITECOPY��	���Խ����κβ������������д������Ļ�����ô����ϵͳ�ͻḴ������ڴ�ҳ�棬�¸��Ʋ����ڵ�ҳ��ı���
	(0x80)					���ͱ�Ϊ��PAGE_EXECUTE_READWRITE��ͬʱԭ�����ڴ��ַ����ҳ�潨����ϵ�����̷��ʱ�����ҳ�档
	PAGE_NOACCESS��			����ڴ��ϵ��κβ�����������Υ����ʡ�
	(0x01)
	PAGE_READONLY��			ֻ��ҳ�棻
	(0x02)
	PAGE_READWRITE��			�ɶ�дҳ�棬����ִ�С�
	(0x04)
	PAGE_WRITECOPY��			���Խ��ж�д���������д������Ļ�����ô����ϵͳ�ͻḴ������ڴ�ҳ�棬�¸��Ƶ�ҳ��ı���
	(0x08)					���ͱ�Ϊ��PAGE_READWRITE��ͬʱԭ�����ڴ��ַ����ҳ�潨����ϵ�����̲����ڷ��ʱ�����ҳ�档


	PAGE_GUARD��				�������ҳ�������һ���쳣��
	(0x100)
	PAGE_NOCACHE��			������ʹ�õ��ڴ汣�����ͣ������豸Ҫ����ѡ�ã�Ӧ�ó���ʹ��������͡�
	(0x200)
	PAGE_WRITECOMBINE��		ͬ�ϡ�
	(0x400)

*/
#include <iomanip>
#include<windows.h>
#include<tchar.h>
#include<Tlhelp32.h>
#include<Psapi.h>
#include <iostream>
#include <TlHelp32.h>
#include <strsafe.h>

using namespace std;

#pragma comment(lib, "psapi.lib")

BOOL ModuleFind(HANDLE hSnap, PVOID lpAddress, wchar_t* NAME)
{
	MODULEENTRY32W me = { sizeof(me) };

	BOOL bFound = FALSE;
	BOOL re = Module32First(hSnap, &me);
	for (; re; re = Module32Next(hSnap, &me))
	{
		if (me.modBaseAddr <= lpAddress && (me.modBaseAddr + me.dwSize) >= lpAddress)
		{
			StringCchPrintf(NAME, MAX_PATH, L"%s", me.szExePath);
			bFound = TRUE;
			break;
		}
	}

	return bFound;
}
void MyVirtualQuery(DWORD dwProcessId)
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);

	HANDLE hProcess = OpenProcess(GENERIC_ALL, FALSE, dwProcessId);

	LPSTR lpAddress = NULL;

	while(lpAddress < reinterpret_cast<char*>(0x80000000))
	{
		MEMORY_BASIC_INFORMATION mbi;

		VirtualQuery(lpAddress ,&mbi, sizeof(mbi));

		cout << "BaseAddress: " << hex << mbi.BaseAddress << endl;
		cout << "AllocationBase: "<< hex<< mbi.AllocationBase << endl;
		cout << "AllocationProtect: "<< hex<< mbi.AllocationProtect << endl;
		cout << "RegionSize: "<< hex<< mbi.RegionSize << endl;
		cout << "State: "<< hex<< mbi.State << endl;
		cout << "Type: "<< hex<< mbi.Type << endl;
		cout << "Protect: "<< hex<< mbi.Protect << endl;


		if (mbi.Type != MEM_PRIVATE)
		{
			wchar_t szName[MAX_PATH];
			if (!ModuleFind(hSnap, mbi.BaseAddress, szName))
			{
				BOOL re = GetMappedFileName(hProcess, mbi.BaseAddress, szName, _countof(szName));
				if (re == 0)
					StringCchPrintf(szName, _countof(szName), L"Unknown");
			}
			wcout << L"ModuleName: " << szName << endl;
		}
		lpAddress += mbi.RegionSize;

		cout << "-----------------------------" << endl;

		getchar();
	}
}

int main(int argc, char* argv[])
{
	MyVirtualQuery(GetCurrentProcessId());
}
