// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <cstdint>

#include "AuthDllspy.h"
#include "util.h"



//extern "C" __declspec(dllexport) void Login(string username,string password)
//{
//    InitAuthspy();
//    clean_login();
//    input_Account(username, password);
//    login();
//}

typedef struct PortPath {
    int port;
    char path[MAX_PATH];
} PortPath_t;


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: {
        //HWND hwnd = GetActiveWindow();
        //MessageBox(hwnd, L"DLL已进入目标进程。", L"信息", MB_ICONINFORMATION);
        //getusernamepassword();
        break;
    }    
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

