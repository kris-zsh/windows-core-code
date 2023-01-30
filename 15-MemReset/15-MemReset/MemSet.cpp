#include<Windows.h>
#include<tchar.h>
#include<stdio.h>


/*

���Ƶ��������ӡ

LPVOID WINAPI VirtualAlloc(
  __in_opt  LPVOID lpAddress,
  __in      SIZE_T dwSize,
  __in      DWORD flAllocationType,
  __in      DWORD flProtect
);

lpAddress:	�����ƶ���ϣ����������ڴ���׵�ַ��ע�⣬�����ַ�������ڴ�������ȵ���������
dwSize:		��Ҫ�����ڴ�Ĵ�С��ע�⣬�����ֵ�������ڴ�ҳ���С����������������ǣ�ϵͳ���Զ�������ҳ���С����������
flAllocationType:	�����ڴ�����ͣ�����Ҫ����һ�������ڴ棬����Ҫ����һ�������ڴ棬���������ڴ���������ڴ棬���Ǹ�
					�Ѿ�����õ������ڴ���������ڴ档

					ע�⣬����һ���÷���MEM_RESET��
						�����÷����߲���ϵͳ������ڴ��е�����,������Ч��������ڴ���Ȼ����ʹ�á�

flProtect:	�ڴ�ҳ��ı�������:
	PAGE_EXECUTE:			ֻ��ִ�е��ڴ�Σ���������ж�д��������ô�ͻ����Υ����ʴ��󣬳���ֹͣ���С�
	PAGE_EXECUTE_READ��		�����Խ���д������������д�����������Υ����ʲ�ֹͣ���С�
	PAGE_EXECUTE_READWRITE��	���Խ����κβ�����
	PAGE_EXECUTE_WRITECOPY��	���Խ����κβ������������д������Ļ�����ô����ϵͳ�ͻḴ������ڴ�ҳ�棬�¸��Ʋ����ڵ�ҳ��ı���
							���ͱ�Ϊ��PAGE_EXECUTE_READWRITE��ͬʱԭ�����ڴ��ַ����ҳ�潨����ϵ�����̷��ʱ�����ҳ�档
	PAGE_NOACCESS��			����ڴ��ϵ��κβ�����������Υ����ʡ�
	PAGE_READONLY��			ֻ��ҳ�棻
	PAGE_READWRITE��			�ɶ�дҳ�棬����ִ�С�
	PAGE_WRITECOPY��			���Խ��ж�д���������д������Ļ�����ô����ϵͳ�ͻḴ������ڴ�ҳ�棬�¸��Ƶ�ҳ��ı���
							���ͱ�Ϊ��PAGE_READWRITE��ͬʱԭ�����ڴ��ַ����ҳ�潨����ϵ�����̲����ڷ��ʱ�����ҳ�档


	PAGE_GUARD��				�������ҳ�������һ���쳣��
	PAGE_NOCACHE��			������ʹ�õ��ڴ汣�����ͣ������豸Ҫ����ѡ�ã�Ӧ�ó���ʹ��������͡�
	PAGE_WRITECOMBINE��		ͬ�ϡ�

VirtualAlloc����ʹ�õı������ԣ�
	PAGE_NOACCESS,PAGE_READWRITE,pAGE_READONLY,PAGE_EXECUTE,PAGE_EXECUTE_READ,PAGE_EXECUTE_READWRITE.
Virtualalloc������ʹ�õı������ԣ�
	PAGE_WRITECOPY,PAGE_EXECUTE_WRITECOPY.
VirtualAllocʱ��ֻ�ܸ��������ڴ�ı������ԣ�
	PAGE_GUARD,PAGE_NOCACHE,PAGE_WRITECOMBINE.


BOOL WINAPI VirtualFree(
  __in  LPVOID lpAddress,
  __in  SIZE_T dwSize,
  __in  DWORD dwFreeType
);

lpAddress��	��Ҫ�ͷ��ڴ���׵�ַ
dwSize��		��Ҫ�ͷ��ڴ�Ĵ�С
			���dwFreeType==MEM_RELEASE����ôdwSize������0����ʱ��VirtualAlloc����������ڴ�ȫ�����ͷţ�
			���dwFreeType==MEM_DECOMMIT����ôlpAddress+dwSize����ڴ�λ���Ǹ�ҳ�棬��ô��Щҳ�����������
			�ڴ潫���ͷŵ���
			���dwFreeType==MEM_DECOMMIT��lpAddressΪVirtualAlloc�������صĵ�ַ����dwSize==0����ô��
			VirtualAlloc����������ڴ��Ӧ�������ڴ汻�ͷš�
dwFreeType��
	MEM_RELEASE��	�ͷ������ڴ�������ڴ棻
	MEM_DECOMMIT:	�ͷ������ڴ�


BOOL WINAPI VirtualProtect(
  __in   LPVOID lpAddress,
  __in   SIZE_T dwSize,
  __in   DWORD flNewProtect,
  __out  PDWORD lpflOldProtect
);

ע�⣺��������ı���������ڴ�ı������ԡ�

*/



DWORD Query()
{
	MEMORY_BASIC_INFORMATION info;

	char* lp = 0;

	HANDLE hProcess = GetCurrentProcess();

	while (lp < (char*)0x80000000)
	{
		VirtualQueryEx(hProcess, lp, &info, sizeof(MEMORY_BASIC_INFORMATION));//VirtualQuery
		lp += info.RegionSize;
		//if(!(info.Type&(MEM_IMAGE|MEM_MAPPED)))
			//continue;
		_tprintf(L"BaseAddress = %X\n", info.BaseAddress);
		_tprintf(L"AllocationBase = %X\n", info.AllocationBase);
		_tprintf(L"AllocationProtect = %X\n", info.AllocationProtect);
		_tprintf(L"RegionSize = %X\n", info.RegionSize);
		_tprintf(L"State =%X\n", info.State);
		_tprintf(L"Type = %X\n", info.Type);
		_tprintf(L"Protect = %X\n", info.Protect);


		_tprintf(L"----------------------------------\n");
		_gettchar();

	}

	return 0;

}




int _tmain()
{


	SYSTEM_INFO info;
	GetSystemInfo(&info);

	_tprintf(L"%X %X\n", info.dwPageSize, info.dwAllocationGranularity);

	PVOID lpVoid = VirtualAlloc(NULL, 0x3000, MEM_RESERVE, PAGE_READWRITE);

	_tprintf(L"%X GetLastError = %d\n", lpVoid, GetLastError());

	_tprintf(L"-------------------------------------------------------------------\n");



	lpVoid = VirtualAlloc(lpVoid, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);


	*((DWORD*)lpVoid) = 100;


	_tprintf(L"%d\n", *((DWORD*)lpVoid));

	LPVOID lpVoid_1 = VirtualAlloc((char*)lpVoid + 0x1000, 0x1000, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	*((DWORD*)lpVoid_1) = 200;

	_tprintf(L"%X %d\n", lpVoid_1, *((DWORD*)lpVoid_1));

	//VirtualFree(lpVoid,0,MEM_RELEASE);ȫ���ͷ������ڴ�������ڴ�
	//VirtualFree(lpVoid,0x1000,MEM_DECOMMIT);//�ͷ�ĳһ�������ڴ�
	//VirtualFree(lpVoid,0,MEM_DECOMMIT);//�ͷ�ȫ�������ڴ档


	LPVOID lpVoid_3 = VirtualAlloc(lpVoid_1, 0x1000, MEM_RESET, PAGE_READWRITE);

	_tprintf(L"lpVoid_3 = %X\n", lpVoid_3);
	*((DWORD*)lpVoid_1) = 800;

	_tprintf(L"%X %d\n", lpVoid_1, *((DWORD*)lpVoid_1));

	_tprintf(L"-------------------------------------------------------------------\n");

	Query();


	_gettchar();

	VirtualFree(lpVoid, 0, MEM_RELEASE);
	return 0;
}