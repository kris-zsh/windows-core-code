#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <thread>
using namespace std;


DWORD WINAPI RunningThreadHandler(PVOID)
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
	for (;;);
	return 0;
}

void RunningThread()
{
	DWORD threadID;
	CloseHandle(CreateThread(nullptr, 0, RunningThreadHandler, nullptr, 0, &threadID));
	wcout << L"running thread Id : " << threadID << endl;
}

DWORD WINAPI LockInfiniteMutexHandler(PVOID pvParam) {

	HANDLE hMutex = (HANDLE)pvParam;
	WaitForSingleObject(hMutex, INFINITE);

	wcout << L"\t threadId " << GetCurrentThreadId() << L"never goes up to here" << endl;
	return(0);
}

void LockInfinite() {

	HANDLE hMutex = CreateMutex(NULL, TRUE, TEXT("InfiniteMutex"));

	DWORD threadID;
	CloseHandle(
		CreateThread(NULL, 0, LockInfiniteMutexHandler, hMutex, 0,
			&threadID));

	wcout << L"Infinite wait on threadId " << threadID << endl;
}
CRITICAL_SECTION se1, se2;

DWORD WINAPI LockCriticalSectionHandler(PVOID szParam)
{
	int i = PtrToInt(szParam);
	if(i == 1)
	{
		EnterCriticalSection(&se1);
		std::this_thread::sleep_for(std::chrono::seconds(10));
		EnterCriticalSection((&se2));
	}
	else
	{
		EnterCriticalSection(&se2);
		std::this_thread::sleep_for(std::chrono::seconds(10));
		EnterCriticalSection((&se1));
	}
	wcout << L"\nLockCriticalSection never reach there" << endl;
	return 0;
}

void LockedCriticalSection()
{
	InitializeCriticalSection(&se1);
	InitializeCriticalSection(&se2);

	DWORD dwThreadID1, dwThreadID2;
	CloseHandle(CreateThread(nullptr, 0, LockCriticalSectionHandler, (PVOID)1, 0, &dwThreadID1));
	CloseHandle(CreateThread(nullptr, 0, LockCriticalSectionHandler, (PVOID)2, 0, &dwThreadID2));

	wcout << "Lock CriticalSection " << endl;
	wcout << "\tthread1 ID " << dwThreadID1 << endl;
	wcout << "\tthread2 ID " << dwThreadID2 << endl;
}
HANDLE hMutex1, hMutex2, hMutex3;

DWORD WINAPI LockMutexHandler(PVOID szParam)
{
	int i = PtrToInt(szParam);
	if (i == 1)
	{
		WaitForSingleObject(hMutex1, INFINITE);
		Sleep(1000 * 10);
		WaitForSingleObject(hMutex2, INFINITE);
	}
	else if(i == 2)
	{
		WaitForSingleObject(hMutex2, INFINITE);
		Sleep(1000 * 10);
		WaitForSingleObject(hMutex3, INFINITE);
	}
	else
	{
		WaitForSingleObject(hMutex3, INFINITE);
		Sleep(1000 * 10);
		WaitForSingleObject(hMutex1, INFINITE);
	}

	wcout << L"\nLockMutexHandler never reach there" << endl;
	return 0;

}
void LockMutex()
{
	hMutex1 = CreateMutex(nullptr, FALSE, L"firstMutex");
	hMutex2 = CreateMutex(nullptr, FALSE, L"secondMutex");
	hMutex3 = CreateMutex(nullptr, FALSE, L"thirdMutex");
	DWORD dwThreadID1, dwThreadID2, dwThreadID3;
	CloseHandle(CreateThread(nullptr, 0, LockMutexHandler, (PVOID)1, 0, &dwThreadID1));
	CloseHandle(CreateThread(nullptr, 0, LockMutexHandler, (PVOID)2, 0, &dwThreadID2));
	CloseHandle(CreateThread(nullptr, 0, LockMutexHandler, (PVOID)3, 0, &dwThreadID3));

	wcout << "LockMutex " << endl;
	wcout << "\tthread1 ID " << dwThreadID1 << endl;
	wcout << "\tthread2 ID " << dwThreadID2 << endl;
	wcout << "\tthread3 ID " << dwThreadID3 << endl;
}
CRITICAL_SECTION abce;

DWORD WINAPI AbandonedCriticalSectionHandler(PVOID szParam)
{
	if (!szParam)
	{
		EnterCriticalSection(&abce);
		wcout << L"\tThreadID " << GetCurrentThreadId() << " has abandoned a CriticalSection" << endl;
	}
	else
	{
		wcout << L"\tThreadID " << GetCurrentThreadId() << L" enter a abandoned CriticalSection" << endl;
		EnterCriticalSection(&abce);
		wcout << '\t' << GetCurrentThreadId() << L" is leaving an abandonned critical section" << endl;
		LeaveCriticalSection(&abce);
		wcout << '\t' << GetCurrentThreadId() << L" is leaved an abandonned critical section" << endl;
	}
	return 0;
}

void AbandonnedCriticalSection()
{
	InitializeCriticalSection(&abce);
	DWORD dwThreadId1, dwThreadId2;
	HANDLE thread;
	CloseHandle(CreateThread(nullptr, 0, AbandonedCriticalSectionHandler, nullptr, 0, &dwThreadId1));

	wcout << L"AbandonedCriticalSection Id " << dwThreadId1 << endl;
	Sleep(1000 * 10);
	thread = CreateThread(nullptr, 0, AbandonedCriticalSectionHandler, (PVOID)L"Ê÷ê»ÐÁ¿àÁË", 0, &dwThreadId2);
	CloseHandle(thread);
}

DWORD WINAPI LockProcessHandler(PVOID szParam)
{
	HANDLE hProcess = (HANDLE)szParam;
	WaitForSingleObject(hProcess, INFINITE);
	CloseHandle(hProcess);
	return 0;
}
void LockProcess()
{
	STARTUPINFOW si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	wchar_t lpCmd[] = L"notepad";

	CreateProcess(nullptr, lpCmd, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
	CloseHandle(pi.hThread);
	DWORD dwThread;
	HANDLE hThread = CreateThread(nullptr, 0, LockProcessHandler, pi.hProcess, 0, &dwThread);
	wcout << L"wait notepad process exit" << endl;
}

DWORD WINAPI LockThreadHandler(PVOID szParam)
{
	HANDLE hParent = szParam;
	WaitForSingleObject(hParent, INFINITE);
	return 0;
}
void LockThread()
{
	HANDLE hParent;
	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &hParent, 0, FALSE, DUPLICATE_SAME_ACCESS);

	HANDLE hHandles[3] = {};
	DWORD dwThreadId[3] = {};
	for (int i = 0; i < sizeof(hHandles) / sizeof(HANDLE); i++)
		hHandles[i] = CreateThread(nullptr, 0, LockThreadHandler, hParent, 0, &dwThreadId[i]);

	wcout << L"LockThread " << endl;
	wcout << L"Thread1ID " << dwThreadId[0]<<endl;
	wcout << L"Thread2ID " << dwThreadId[1]<<endl;
	wcout << L"Thread3ID " << dwThreadId[2]<<endl;
	for (int i = 0; i < 3; i++)
		CloseHandle(hHandles[i]);
}
void TestDeadLock()
{
	wcout << L"TestDeadLock ProcessID = " << GetCurrentProcessId() << endl;
	wcout << "-----------------------------------" << endl;

	RunningThread();
	LockInfinite();
	LockedCriticalSection();
	LockMutex();
	AbandonnedCriticalSection();
	LockProcess();
	LockThread();
}
int main(int argc, char* argv[])
{
	std::wcout.imbue(std::locale("chs"));
	TestDeadLock();
}
