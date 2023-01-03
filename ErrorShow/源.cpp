#include <windows.h>
#include <iostream>
#include <tchar.h>
#include <WindowsX.h>

int main(int argc, char* argv[])
{
    std::wcout.imbue(std::locale("chs"));

    DWORD dwError;
    std::cin >> dwError;

    HLOCAL hlocal = NULL;

    DWORD systemLocale = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL);

    BOOL fOk = FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
        FORMAT_MESSAGE_ALLOCATE_BUFFER,
        NULL, dwError, systemLocale,
        (PTSTR)&hlocal, 0, NULL);

    if (!fOk) {
        // Is it a network-related error?
        HMODULE hDll = LoadLibraryEx(TEXT("netmsg.dll"), NULL,
            DONT_RESOLVE_DLL_REFERENCES);

        if (hDll != NULL) {
            fOk = FormatMessage(
                FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS |
                FORMAT_MESSAGE_ALLOCATE_BUFFER,
                hDll, dwError, systemLocale,
                (PTSTR)&hlocal, 0, NULL);
            FreeLibrary(hDll);
        }
    }
    std::wcout << "dwErrorMessage: " << (PCTSTR)LocalLock(hlocal) << std::endl;
    LocalFree(hlocal);
}

