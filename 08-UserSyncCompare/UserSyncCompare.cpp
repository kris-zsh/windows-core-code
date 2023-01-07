#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <chrono>

#define MAX_COUNT 10000

using namespace std;
using namespace std::chrono;

using op = void (*)();
volatile long g_val = 0;

HANDLE hMutex;
CRITICAL_SECTION se;
SRWLOCK srwlock;

DWORD WINAPI TransferStation(PVOID szParam)
{
	op fun = (op)szParam;
	for (int i = 0; i < MAX_COUNT; i++)
		fun();
	return 0;
}
void EnsureConcurrentOpertion(int threadCount, op fun, const std::wstring& funName)
{
	HANDLE* hHandles = new HANDLE[threadCount];

	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
	auto start = chrono::system_clock::now();

	for (int i = 0; i < threadCount; i++)
		hHandles[i] = CreateThread(nullptr, 0, TransferStation, fun, 0, nullptr);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);

	WaitForMultipleObjects(threadCount, hHandles, TRUE, INFINITE);
	auto end = chrono::system_clock::now();

	wcout << funName << " fun cost time: " << chrono::duration_cast<chrono::microseconds>(end - start).count() << endl;
}

void VolatileRead()
{
	LONG l = g_val;
}

void VolatileWrite()
{
	g_val = 0;
}

void VolatileCallBack()
{
	InterlockedIncrement(&g_val);
}

void CriticalSectionCallBack()
{
	EnterCriticalSection(&se);
	g_val = 0;
	LeaveCriticalSection(&se);
}

void SRWLockReadCallBack()
{
	AcquireSRWLockShared(&srwlock);
	g_val = 0;
	ReleaseSRWLockShared(&srwlock);
}

void SRWLockWriteCallBack()
{
	AcquireSRWLockExclusive(&srwlock);
	g_val = 0;
	ReleaseSRWLockExclusive(&srwlock);
}

void MutexCallBack()
{
	WaitForSingleObject(hMutex, INFINITE);
	g_val = 0;
	ReleaseMutex(hMutex);
}

int main(int argc, char* argv[])
{
	std::wcout.imbue(std::locale("chs"));
	for(int i = 1; i <= 4; i *= 2)
	{
		EnsureConcurrentOpertion(i, VolatileRead, L"volatile read");
		EnsureConcurrentOpertion(i, VolatileWrite, L"volatile write");
		EnsureConcurrentOpertion(i, VolatileCallBack, L"volatile callback");

		InitializeCriticalSection(&se);
		EnsureConcurrentOpertion(i, CriticalSectionCallBack, L"CriticalSection callback");
		DeleteCriticalSection(&se);

		InitializeSRWLock(&srwlock);
		EnsureConcurrentOpertion(i, SRWLockReadCallBack, L"SWRLockRead callback");
		EnsureConcurrentOpertion(i, SRWLockWriteCallBack, L"SWRLockWrite callback");

		hMutex = CreateMutex(nullptr, FALSE, nullptr);
		EnsureConcurrentOpertion(i, MutexCallBack, L"Mutex callback");

		wcout << endl;
	}
}
