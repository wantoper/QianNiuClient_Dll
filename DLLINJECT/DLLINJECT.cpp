#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <tchar.h>
#include "psapi.h"
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
HANDLE ApcInjectDll(DWORD dwPid,const wchar_t* szDllName)
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
    WriteProcessMemory(hProcess, pDllAddr, szDllName, nDllLen, NULL);

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
    return hProcess;
}

// 从进程中查找某个模块
HMODULE GetTargetModuleBase(HANDLE process, string dll)
{
    DWORD cbNeeded;
    HMODULE moduleHandleList[512];
    BOOL ret = EnumProcessModulesEx(process, moduleHandleList, sizeof(moduleHandleList), &cbNeeded, LIST_MODULES_64BIT);
    if (!ret) {
        MessageBox(NULL, L"获取模块失败", L"GetTargetModuleBase", 0);
        return NULL;
    }

    if (cbNeeded > sizeof(moduleHandleList)) {
        MessageBox(NULL, L"模块数量过多", L"GetTargetModuleBase", 0);
        return NULL;
    }
    DWORD processCount = cbNeeded / sizeof(HMODULE);

    char moduleName[32];
    for (DWORD i = 0; i < processCount; i++) {
        GetModuleBaseNameA(process, moduleHandleList[i], moduleName, 32);
        if (!strncmp(dll.c_str(), moduleName, dll.size())) {
            return moduleHandleList[i];
        }
    }
    return NULL;
}

// 获取一个dll内funcName函数的偏移    dllPath需要导出该函数
static UINT64 GetFuncOffset(LPCWSTR dllPath, LPCSTR funcName)
{
    HMODULE dll = LoadLibrary(dllPath);
    if (dll == NULL) {
        MessageBox(NULL, L"获取 DLL 失败", L"GetFuncOffset", 0);
        return 0;
    }

    LPVOID absAddr = GetProcAddress(dll, funcName);
    std::wcout << L"GetFuncOffset: " << absAddr << std::endl;
    UINT64 offset = (UINT64)absAddr - (UINT64)dll;
    FreeLibrary(dll);

    return offset;
}

// 远程调用模块函数 进程,dll全路径,dll的在进程中的偏移,函数名,参数,参数大小
bool CallDllFuncEx(HANDLE process, LPCWSTR dllPath, HMODULE dllBase, LPCSTR funcName, LPVOID parameter, size_t sz,
    LPDWORD ret)
{

    UINT64 offset = GetFuncOffset(dllPath, funcName);
    if (offset == 0) {
        return false;
    }
    UINT64 pFunc = (UINT64)dllBase + GetFuncOffset(dllPath, funcName);
    
    if (pFunc <= (UINT64)dllBase) {
        return false;
    }
    

    LPVOID pRemoteAddress = VirtualAllocEx(process, NULL, sz, MEM_COMMIT, PAGE_READWRITE);
    if (pRemoteAddress == NULL) {
        MessageBox(NULL, L"申请内存失败", L"CallDllFuncEx", 0);
        return NULL;
    }

    WriteProcessMemory(process, pRemoteAddress, parameter, sz, NULL);

    HANDLE hThread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)pFunc, pRemoteAddress, 0, NULL);
    if (hThread == NULL) {
        VirtualFree(pRemoteAddress, 0, MEM_RELEASE);
        MessageBox(NULL, L"remote call error", L"CallDllFuncEx", 0);
        return false;
    }
    WaitForSingleObject(hThread, INFINITE);
    VirtualFree(pRemoteAddress, 0, MEM_RELEASE);
    if (ret != NULL) {
        GetExitCodeThread(hThread, ret);
    }

    CloseHandle(hThread);
    return true;
}


typedef struct QNParam {
    wchar_t username[MAX_PATH];
    wchar_t password[MAX_PATH];
} QNParam_t;



int main(int argc, char* argv[])
{
    //SetConsoleOutputCP(65001);
    //std::wcout << L"Start... " << std::endl;
    DWORD pid = FindProcessID(L"AliWorkbench.exe");
    std::wcout << L"PID: " << pid << std::endl;
    const wchar_t* dllpath = L"C:\\Users\\Administrator\\Desktop\\Wantoper-Hexo\\QianNiuClient_Dll\\x64\\Release\\QianNiuDll.dll";
    HANDLE hProcess = ApcInjectDll(pid, dllpath);


    Sleep(2000);
    HMODULE dllBase = GetTargetModuleBase(hProcess, "QianNiuDll.dll");
    QNParam_t param = { 0 };
    wcscpy_s(param.username, MAX_PATH, L"达欣源灯具直销店:小月");
    wcscpy_s(param.password, MAX_PATH, L"dajiahao1398");

    std::wcout << L"dllBase: " << dllBase << std::endl;
    bool is = CallDllFuncEx(hProcess, dllpath, dllBase, "InitSpy", (LPVOID)&param, sizeof(QNParam_t), NULL);
    return 0;
}