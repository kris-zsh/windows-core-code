/*

typedef struct _MEMORY_BASIC_INFORMATION {
  PVOID  BaseAddress;
  PVOID  AllocationBase;
  DWORD  AllocationProtect;//虚拟地址内存的保护属性
  SIZE_T RegionSize;
  DWORD  State;
  DWORD  Protect;//物理内存的保护属性
  DWORD  Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;


BaseAddress:	该段的基地址
AllocationBase：	使用VirtualAlloc分配内存时，返回的内存地址，也就是分配内存的首地址
RegionSize：		段大小
State：			
				MEM_COMMIT：		虚拟内存已经被分配，物理内存已经被分配	（0x1000）
				MEM_RESERVE：	虚拟内存被分配，但没有物理内存与之对应	（0x2000）
				MEM_FREE:		虚拟内存没有被分配					（0x10000）

Type：			MEM_IMAGE:		一般是将EXE或者DLL文件影射到内存中的内存类型	（0x1000000）
				MEM_MAPPED:		一般是内存影射文件影射到这个内存段				（0x40000）
				MEM_PRIVATE:	私有内存									（0x20000）


AllocationProtect/Protect:	内存页面的保护类型:

	PAGE_EXECUTE:			只能执行的内存段，如果发现有读写操作，那么就会出现违规访问错误，程序停止运行。
	(0x10)
	PAGE_EXECUTE_READ：		不可以进行写入操作，如果有写入操作，出现违规访问并停止运行。
	(0x20)
	PAGE_EXECUTE_READWRITE：	可以进行任何操作。
	(0x40)
	PAGE_EXECUTE_WRITECOPY：	可以进行任何操作，但如果是写入操作的话，那么操作系统就会复制这段内存页面，新复制不能在的页面的保护
	(0x80)					类型变为：PAGE_EXECUTE_READWRITE，同时原虚拟内存地址和新页面建立关系，进程访问被复制页面。
	PAGE_NOACCESS：			这段内存上的任何操作都会引发违规访问。
	(0x01)
	PAGE_READONLY：			只读页面；
	(0x02)
	PAGE_READWRITE：			可读写页面，不能执行。
	(0x04)
	PAGE_WRITECOPY：			可以进行读写，但如果是写入操作的话，那么操作系统就会复制这段内存页面，新复制的页面的保护
	(0x08)					类型变为：PAGE_READWRITE，同时原虚拟内存地址和新页面建立关系，进程不能在访问被复制页面。


	PAGE_GUARD：				访问这个页面会引发一个异常。
	(0x100)
	PAGE_NOCACHE：			驱动中使用的内存保护类型，根据设备要求来选用，应用程序不使用这个类型。
	(0x200)
	PAGE_WRITECOMBINE：		同上。
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
