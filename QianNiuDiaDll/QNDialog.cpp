// QNDialog.cpp: 实现文件
//

#include "pch.h"
#include "QianNiu.h"
#include "QNDialog.h"
#include "afxdialogex.h"
#include <Psapi.h>
#include <windows.h>
#include <string>
#include <vector>
// QNDialog 对话框

IMPLEMENT_DYNAMIC(QNDialog, CDialogEx)

QNDialog::QNDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
{

}

QNDialog::~QNDialog()
{
}

void QNDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDITa, edit_username);
	DDX_Control(pDX, IDC_EDITB, edit_password);
}


BEGIN_MESSAGE_MAP(QNDialog, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &QNDialog::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &QNDialog::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &QNDialog::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &QNDialog::OnBnClickedButton4)
END_MESSAGE_MAP()


void QNDialog::OnBnClickedButton1()
{
	HMODULE hModule = GetModuleHandle(_T("AliAuthSDK.dll"));
	QWORD base_address = (QWORD)hModule;

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


	CString str;
	str.Format(L"%s", username);
	edit_username.SetWindowTextW(username);
	str.Format(L"%s", password);
	edit_password.SetWindowTextW(password);
}



typedef QWORD (*Loginbtn_Click)(QWORD, QWORD, QWORD, QWORD);

extern "C" void calc(QWORD a, QWORD b, QWORD c, QWORD d, QWORD e);
void QNDialog::OnBnClickedButton2()
{
	HMODULE hModule = GetModuleHandle(_T("AliAuthSDK.dll"));
	QWORD base_address = (QWORD)hModule;

	//RCX=  [AliAuthSDK.dll+2D2250]
	//RAX= [RCX]--11002F			
	//QWORD RCX = *(QWORD*)(base_address + 0x2D2250);
	//QWORD RAX = (*(QWORD*)RCX) - 0x10CEB0;
	//calc(RCX, 0x000000000000000F, 0x00000000FFFFFFFF, 0x0000000000000001, RAX);  //旧方法


	QWORD RCX = *(QWORD*)(base_address + 0x2D2250);
	Loginbtn_Click click = (Loginbtn_Click)((*(QWORD*)RCX) - 0x10CEB0);
	click(RCX, 0x000000000000000F, 0x00000000FFFFFFFF, 0x0000000000000001);			//新方法
}


void QNDialog::OnBnClickedButton3()
{
	HMODULE hModule = GetModuleHandle(_T("AliAuthSDK.dll"));
	QWORD base_address = (QWORD)hModule;

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

	CString usernameFromEdit, passwordFromEdit;
	edit_username.GetWindowTextW(usernameFromEdit);
	edit_password.GetWindowTextW(passwordFromEdit);
	wcscpy_s(username, 256, usernameFromEdit);
	wcscpy_s(password, 256, passwordFromEdit); 
}


typedef QWORD(*Delaccount_Click)(QWORD, QWORD, QWORD, QWORD);
void QNDialog::OnBnClickedButton4()
{
	HMODULE hModule = GetModuleHandle(_T("AliAuthSDK.dll"));
	QWORD base_address = (QWORD)hModule;

	//RCX = "AliAuthSDK.dll" + 002D2250 30 1C8 88 130 90 90
	QWORD RCX = *(QWORD*)(base_address + 0x2D2250);
	RCX = *(QWORD*)(RCX + 0x30);
	RCX = *(QWORD*)(RCX + 0x1C8);
	RCX = *(QWORD*)(RCX + 0X88);
	RCX = *(QWORD*)(RCX + 0X130);
	RCX = *(QWORD*)(RCX + 0X90);
	RCX = *(QWORD*)(RCX + 0X90);



	//登陆方式 "AliAuthSDK.dll"+002D2250 30 1C8 88 238           写入0  普通登录 写入1 快速登陆
	QWORD signallogin = *(QWORD*)(base_address + 0x2D2250);
	signallogin = *(QWORD*)(signallogin + 0x30);
	signallogin = *(QWORD*)(signallogin + 0x1C8);
	signallogin = *(QWORD*)(signallogin + 0X88);
	BYTE* login = (BYTE*)(signallogin + 0X238);
	*login = 0;
	
	//call= [[AliAuthSDK.dll + 2D2250]] - C6F90
	QWORD call = *(QWORD*)(base_address + 0x2D2250);
	call = (*(QWORD*)(call)) - 0xC6F90;
	Loginbtn_Click click = (Loginbtn_Click)(call);
	click(RCX, 0x000000000000000F, 0x00000000FFFFFFFF, 0x0000000000000001);

	//修改账号
	OnBnClickedButton3();
	//登录
	OnBnClickedButton2();
}
