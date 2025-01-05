#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <tchar.h>
#include <string>
using namespace std;

// 传入进程名称返回该进程PID
DWORD FindProcessID(LPCTSTR szProcessName)
{
    DWORD dwPID = 0xFFFFFFFF;
    HANDLE hSnapShot = INVALID_HANDLE_VALUE;
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
    Process32First(hSnapShot, &pe);
    do
    {
        if (!_wcsicmp(szProcessName, (LPCTSTR)pe.szExeFile))
        {
            dwPID = pe.th32ProcessID;
            break;
        }
    } while (Process32Next(hSnapShot, &pe));
    CloseHandle(hSnapShot);

    return dwPID;
}

// APC注入
BOOL ApcInjectDll(DWORD dwPid, wchar_t* szDllName)
{
    // 得到文件完整路径长度
    int nDllLen = (wcslen(szDllName) + 1) * sizeof(wchar_t);

    // 打开目标进程
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
    if (hProcess == NULL)
    {
        return FALSE;
    }

    // 在对端开辟内存空间
    PVOID pDllAddr = VirtualAllocEx(hProcess, NULL, nDllLen, MEM_COMMIT, PAGE_READWRITE);
    if (pDllAddr == NULL)
    {
        CloseHandle(hProcess);
        return FALSE;
    }

    // 将DLL路径写入目标进程
#ifdef _WIN64
    size_t dwWriteNum = 0;
    WriteProcessMemory(hProcess, pDllAddr, szDllName, nDllLen, &dwWriteNum);
#else
    DWORD dwWriteNum = 0;
    WriteProcessMemory(hProcess, pDllAddr, szDllName, nDllLen, &dwWriteNum);
#endif
    CloseHandle(hProcess);

    THREADENTRY32 te = { 0 };
    te.dwSize = sizeof(THREADENTRY32);

    // 得到线程快照
    HANDLE handleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (INVALID_HANDLE_VALUE == handleSnap)
    {
        CloseHandle(hProcess);
        return FALSE;
    }

    // 得到LoadLibraryW函数的地址
    FARPROC pFunAddr = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
    DWORD dwRet = 0;

    // 得到第一个线程
    if (Thread32First(handleSnap, &te))
    {
        do
        {
            // 进程ID对比是否为传入的进程
            if (te.th32OwnerProcessID == dwPid)
            {
                // 打开线程，得到线程句柄
                HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);
                if (hThread)
                {
                    // 向线程插入APC队列
                    dwRet = QueueUserAPC((PAPCFUNC)pFunAddr, hThread, (ULONG_PTR)pDllAddr);

                    // 关闭句柄
                    CloseHandle(hThread);
                }
            }
            // 循环下一个线程
        } while (Thread32Next(handleSnap, &te));
    }

    // 关闭句柄
    CloseHandle(handleSnap);
    return TRUE;
}

int main(int argc, char* argv[])
{
    SetConsoleOutputCP(65001);
    std::wcout << L"Start... " << std::endl;
    DWORD pid = FindProcessID(L"AliWorkbench.exe");
    std::wcout << L"PID: " << pid << std::endl;

    bool flag = ApcInjectDll(pid, L"C:\\Users\\Administrator\\Desktop\\项目\\QianNiuClient_Dll\\x64\\Release\\QianNiuDll.dll");
    std::wcout << L"Status: " << flag << std::endl;
    return 0;
}