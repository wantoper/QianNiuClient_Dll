#include "pch.h"
#include <cstdint>
#include <string>
#include "util.h"
using namespace std;


#define AuthDll L"AliAuthSDK.dll"

typedef uint64_t QWORD;
typedef QWORD(*Delaccount_Click)(QWORD, QWORD, QWORD, QWORD);
typedef QWORD(*Login_Click)(QWORD, QWORD, QWORD, QWORD);


QWORD g_AuthDllWinDllAddr = 0;

void InitAuthspy() {
    HMODULE hModule = GetModuleHandle(AuthDll);
    g_AuthDllWinDllAddr = (QWORD)hModule;
}

void clean_login() {      
    //RCX = "AliAuthSDK.dll" + 002D2250 30 1C8 88 130 90 90
    QWORD RCX = *(QWORD*)(g_AuthDllWinDllAddr + 0x2D2250);
    RCX = *(QWORD*)(RCX + 0x30);
    RCX = *(QWORD*)(RCX + 0x1C8);
    RCX = *(QWORD*)(RCX + 0X88);
    RCX = *(QWORD*)(RCX + 0X130);
    RCX = *(QWORD*)(RCX + 0X90);
    RCX = *(QWORD*)(RCX + 0X90);

    //登陆方式 "AliAuthSDK.dll"+002D2250 30 1C8 88 238           写入0  普通登录 写入1 快速登陆
    QWORD signallogin = *(QWORD*)(g_AuthDllWinDllAddr + 0x2D2250);
    signallogin = *(QWORD*)(signallogin + 0x30);
    signallogin = *(QWORD*)(signallogin + 0x1C8);
    signallogin = *(QWORD*)(signallogin + 0X88);
    BYTE* login = (BYTE*)(signallogin + 0X238);
    *login = 0;

    //call= [[AliAuthSDK.dll + 2D2250]] - C6F90
    QWORD call = *(QWORD*)(g_AuthDllWinDllAddr + 0x2D2250);
    call = (*(QWORD*)(call)) - 0xC6F90;
    Delaccount_Click click = (Delaccount_Click)(call);
    click(RCX, 0x000000000000000F, 0x00000000FFFFFFFF, 0x0000000000000001);
}

void getusernamepassword() {
    HMODULE hModuledll = GetModuleHandle(AuthDll);
    QWORD base_address = (QWORD)hModuledll;

    QWORD tmp;
    tmp = *(QWORD*)(base_address + 0x002D2250);
    tmp = *(QWORD*)(tmp + 0x30);
    tmp = *(QWORD*)(tmp + 0x1C8);
    tmp = *(QWORD*)(tmp + 0x88);

    QWORD usernametmp;
    usernametmp = *(QWORD*)(tmp + 0x110);
    wchar_t* username = (wchar_t*)(usernametmp + 0x230);


    QWORD passwordtmp;
    passwordtmp = *(QWORD*)(tmp + 0x130);
    wchar_t* password = (wchar_t*)(passwordtmp + 0x230);

    HWND hwnd = GetActiveWindow();
    MessageBox(hwnd, username, L"信息", MB_ICONINFORMATION);
    MessageBox(hwnd, password, L"信息", MB_ICONINFORMATION);
}

void input_Account(wchar_t* username, wchar_t* password) {
    QWORD tmp;
    tmp = *(QWORD*)(g_AuthDllWinDllAddr + 0x002D2250);
    tmp = *(QWORD*)(tmp + 0x30);
    tmp = *(QWORD*)(tmp + 0x1C8);
    tmp = *(QWORD*)(tmp + 0x88);

    QWORD usernametmp;
    usernametmp = *(QWORD*)(tmp + 0x110);
    wchar_t* usernamep = (wchar_t*)(usernametmp + 0x230);

    QWORD passwordtmp;
    passwordtmp = *(QWORD*)(tmp + 0x130);
    wchar_t* passwordp = (wchar_t*)(passwordtmp + 0x230);


    wcscpy_s(usernamep, 256, username);
    wcscpy_s(passwordp, 256, password);

    //wcscpy_s(usernamep, 256, username);
    //wcscpy_s(passwordp, 256, password);
}

void login() {
    QWORD RCX = *(QWORD*)(g_AuthDllWinDllAddr + 0x2D2250);
    Login_Click login = (Login_Click)((*(QWORD*)RCX) - 0x10CEB0);
    login(RCX, 0x000000000000000F, 0x00000000FFFFFFFF, 0x0000000000000001);
}

typedef struct QNParam {
    wchar_t username[MAX_PATH];
    wchar_t password[MAX_PATH];
} QNParam_t;

void InitSpy(LPVOID args)
{
    wchar_t version[16] = { 0 };
    QNParam* pp = (QNParam*)args;

    //HWND hwnd = GetActiveWindow();
    //MessageBox(hwnd, pp->username, L"信息", MB_ICONINFORMATION);
    //MessageBox(hwnd,L"Message!!!!!!!!", L"信息", MB_ICONINFORMATION);

    InitAuthspy();
    clean_login();
    input_Account(pp->username,pp->password);
    login();
}