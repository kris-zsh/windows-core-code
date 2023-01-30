#include<Windows.h>
#include<tchar.h>
#include<stdio.h>


/*

复制到剪贴板打印

LPVOID WINAPI VirtualAlloc(
  __in_opt  LPVOID lpAddress,
  __in      SIZE_T dwSize,
  __in      DWORD flAllocationType,
  __in      DWORD flProtect
);

lpAddress:	用来制定你希望获得虚拟内存的首地址，注意，这个地址必须是内存分配粒度的整数倍。
dwSize:		需要分配内存的大小，注意，这个数值必须是内存页面大小的整数倍，如果不是，系统会自动调整到页面大小的整数倍。
flAllocationType:	分配内存的类型，你是要分配一段虚拟内存，还是要分配一段虚拟内存，并给虚拟内存分配物理内存，还是给
					已经分配好的虚拟内存分配物理内存。

					注意，还有一种用法：MEM_RESET：
						这种用法告诉操作系统，这段内存中的数据,不再有效，但这段内存依然可以使用。

flProtect:	内存页面的保护类型:
	PAGE_EXECUTE:			只能执行的内存段，如果发现有读写操作，那么就会出现违规访问错误，程序停止运行。
	PAGE_EXECUTE_READ：		不可以进行写入操作，如果有写入操作，出现违规访问并停止运行。
	PAGE_EXECUTE_READWRITE：	可以进行任何操作。
	PAGE_EXECUTE_WRITECOPY：	可以进行任何操作，但如果是写入操作的话，那么操作系统就会复制这段内存页面，新复制不能在的页面的保护
							类型变为：PAGE_EXECUTE_READWRITE，同时原虚拟内存地址和新页面建立关系，进程访问被复制页面。
	PAGE_NOACCESS：			这段内存上的任何操作都会引发违规访问。
	PAGE_READONLY：			只读页面；
	PAGE_READWRITE：			可读写页面，不能执行。
	PAGE_WRITECOPY：			可以进行读写，但如果是写入操作的话，那么操作系统就会复制这段内存页面，新复制的页面的保护
							类型变为：PAGE_READWRITE，同时原虚拟内存地址和新页面建立关系，进程不能在访问被复制页面。


	PAGE_GUARD：				访问这个页面会引发一个异常。
	PAGE_NOCACHE：			驱动中使用的内存保护类型，根据设备要求来选用，应用程序不使用这个类型。
	PAGE_WRITECOMBINE：		同上。

VirtualAlloc可以使用的保护属性：
	PAGE_NOACCESS,PAGE_READWRITE,pAGE_READONLY,PAGE_EXECUTE,PAGE_EXECUTE_READ,PAGE_EXECUTE_READWRITE.
Virtualalloc不可以使用的保护属性：
	PAGE_WRITECOPY,PAGE_EXECUTE_WRITECOPY.
VirtualAlloc时，只能赋予物理内存的保护属性：
	PAGE_GUARD,PAGE_NOCACHE,PAGE_WRITECOMBINE.


BOOL WINAPI VirtualFree(
  __in  LPVOID lpAddress,
  __in  SIZE_T dwSize,
  __in  DWORD dwFreeType
);

lpAddress：	需要释放内存的首地址
dwSize：		需要释放内存的大小
			如果dwFreeType==MEM_RELEASE，那么dwSize必须是0，此时由VirtualAlloc函数分配的内存全部被释放，
			如果dwFreeType==MEM_DECOMMIT，那么lpAddress+dwSize这段内存位于那个页面，那么这些页面的所有物理
			内存将被释放掉。
			如果dwFreeType==MEM_DECOMMIT，lpAddress为VirtualAlloc函数返回的地址，而dwSize==0，那么由
			VirtualAlloc分配的整段内存对应的物理内存被释放。
dwFreeType：
	MEM_RELEASE：	释放物理内存和虚拟内存；
	MEM_DECOMMIT:	释放物理内存


BOOL WINAPI VirtualProtect(
  __in   LPVOID lpAddress,
  __in   SIZE_T dwSize,
  __in   DWORD flNewProtect,
  __out  PDWORD lpflOldProtect
);

注意：这个函数改变的是物理内存的保护属性。

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

	//VirtualFree(lpVoid,0,MEM_RELEASE);全部释放物理内存和虚拟内存
	//VirtualFree(lpVoid,0x1000,MEM_DECOMMIT);//释放某一段物理内存
	//VirtualFree(lpVoid,0,MEM_DECOMMIT);//释放全部物理内存。


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