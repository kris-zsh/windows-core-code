#include <Windows.h>
#include <iostream>

using std::cout;
using std::wcout;
using std::endl;

BOOL UseDefaultHeap()
{
	enum { HEAP_MAX = 1024 };
	LPVOID lpHeap = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, HEAP_MAX);
	if (!lpHeap)
	{
		cout << "HeapAlloc Failed GLE: " << GetLastError() << endl;
		return 0;
	}

	*(DWORD*)lpHeap = 4;
	cout << *(DWORD*)lpHeap << endl;
	HeapFree(GetProcessHeap(), 0, lpHeap);
	return 0;
}

BOOL UseMyHeap()
{
	enum{HEAP_MAX = 1024};

	HANDLE hHeap = HeapCreate(0, 0, 0);
	LPVOID lpHeap = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, HEAP_MAX);
	if (!lpHeap)
	{
		cout << "HeapAlloc Failed GLE: " << GetLastError() << endl;
		return 0;
	}

	*(DWORD*)lpHeap = 4;
	cout << *(DWORD*)lpHeap << endl;
	HeapFree(GetProcessHeap(), 0, lpHeap);

	HeapDestroy(hHeap);
}

class CSomeAlloc
{
public:
	static HANDLE hHeap;
	static DWORD dwUseHeapCnt;
	void* operator new(std::size_t size);
	void operator delete(void* lpAdd);
};

HANDLE CSomeAlloc::hHeap = nullptr;
DWORD CSomeAlloc::dwUseHeapCnt = 0;

void* CSomeAlloc::operator new(std::size_t size)
{
	if(!hHeap)
	{
		hHeap = HeapCreate(0, 0, 0);
		if(!hHeap)
		{
			cout << "CreateHeap Failed GLE: " << GetLastError() << endl;
			return nullptr;
		}
	}

	CSomeAlloc* lpAlloc = static_cast<CSomeAlloc*>(HeapAlloc(hHeap, HEAP_ZERO_MEMORY, size));
	if(!lpAlloc)
	{
		cout << "HeapAlloc failed GLE: " << GetLastError() << endl;
		return nullptr;
	}
	dwUseHeapCnt++;
	cout << "CreateSpace success" << endl;
	return lpAlloc;
}

void CSomeAlloc::operator delete(void* lpAdd)
{
	if (!lpAdd)
		return;
	if(!HeapFree(hHeap, 0, lpAdd))
	{
		cout << "HeapFree failed GLE: " << GetLastError() << endl;
		return;
	}

	if(--dwUseHeapCnt == 0)
	{
		HeapDestroy(hHeap);
		hHeap = nullptr;
	}
	cout << "DestroySpace success" << endl;
}

int main(int argc, char* argv[])
{
	CSomeAlloc* cs = new CSomeAlloc;
	delete cs;
}
