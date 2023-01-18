#include <algorithm>
#include <iostream>
#include <string>
#include <tchar.h>
#include <strsafe.h>
#include <vector>
#include <windows.h>

using namespace std;

typedef struct
{
	BOOL isUsed;
	BYTE bOtherData[2048 - sizeof(BOOL)];
} SOMEDATA, *PSOMEDATA;


DWORD g_PageSize = 0;

PSOMEDATA g_pSomeData = nullptr;

void QueryMemoryUse(vector<int>& useArr)
{
	for (int i = 0; i < 50; i++)
	{
		MEMORY_BASIC_INFORMATION mbi;
		VirtualQuery(&g_pSomeData[i], &mbi, sizeof(mbi));
		if (mbi.State == MEM_COMMIT)
		{
			wcout << i << L"Region already is commit used: " << g_pSomeData[i].isUsed << endl;
			if (g_pSomeData[i].isUsed == TRUE)
				useArr.push_back(i);
		}
	}
}

void GarbageCollect(PSOMEDATA g_pSomeData, DWORD dwNum, DWORD dwSturctSize)
{
	DWORD dwMaxPage = dwNum * dwSturctSize / g_PageSize;

	for (DWORD page = 0; page < dwMaxPage; page++)
	{
		BOOL bAllocMem = FALSE;
		DWORD dwIdx = g_PageSize / dwSturctSize * page;
		DWORD dwIdxLast = dwIdx + g_PageSize / dwSturctSize;

		for (size_t i = dwIdx; i < dwIdxLast; i++)
		{
			MEMORY_BASIC_INFORMATION mbi;
			VirtualQuery(&g_pSomeData[i], &mbi, sizeof(mbi));

			bAllocMem = (mbi.State == MEM_COMMIT && g_pSomeData[i].isUsed == TRUE);
			if (bAllocMem)
			{
				break;
			}
		}

		if (!bAllocMem)
		{
			wcout << L"page " << page << L" will free" << endl;
			VirtualFree(&g_pSomeData[dwIdx], dwSturctSize, MEM_DECOMMIT);
		}
	}
}

int main(int argc, char* argv[])
{
	std::wcout.imbue(std::locale("chs"));

	SYSTEM_INFO info;
	GetSystemInfo(&info);
	g_PageSize = info.dwPageSize;
	wcout << L"PageSize: " << g_PageSize / 1024 << "K" << endl;

	g_pSomeData = static_cast<PSOMEDATA>(VirtualAlloc(nullptr, sizeof(SOMEDATA) * 50, MEM_RESERVE, PAGE_READWRITE));
	if (!g_pSomeData)
	{
		wcout << L"Reverse VitrualMemory failed " << GetLastError() << endl;
	}

	DWORD dwIndex;
	wcout << L"请输入 0~50的数字 一共十次" << endl;

	for (int i = 0; i < 10;)
	{
		cin >> dwIndex;
		if (dwIndex >= 50)
		{
			wcout << dwIndex << "number > 50 over region range,please re-choose" << endl;
			continue;
		}

		MEMORY_BASIC_INFORMATION mbi;
		VirtualQuery(&g_pSomeData[dwIndex], &mbi, sizeof(mbi));
		if (mbi.State == MEM_COMMIT && g_pSomeData[dwIndex].isUsed == TRUE)
		{
			wcout << dwIndex << L" Region already commit please choose other number" << endl;
			continue;
		}

		VirtualAlloc(&g_pSomeData[dwIndex], sizeof(SOMEDATA), MEM_COMMIT, PAGE_READWRITE);
		g_pSomeData[dwIndex].isUsed = TRUE;

		wcout << dwIndex << L"Region apply success ,choose next number" << endl;
		i++;
	}

	vector<int> useArr;
	QueryMemoryUse(useArr);

	for_each(useArr.begin(), useArr.end(), [](int a)
	{
		wcout << a << " ";
	});

	wcout << endl;
	wcout << L"选择五个数 然后清理" << endl;

	for (int i = 0; i < 5;)
	{
		cin >> dwIndex;
		if (find(useArr.begin(), useArr.end(), dwIndex) == useArr.end())
		{
			wcout << dwIndex << " number is not range ,please rechoose" << endl;
			continue;
		}
		g_pSomeData[dwIndex].isUsed = FALSE;
		i++;
	}

	GarbageCollect(g_pSomeData, 50, sizeof(SOMEDATA));

	VirtualFree(g_pSomeData, 0, MEM_RELEASE);
}
