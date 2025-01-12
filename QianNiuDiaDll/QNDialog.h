#pragma once
#include <afxdialogex.h>

// QNDialog 对话框

class QNDialog : public CDialogEx
{
	DECLARE_DYNAMIC(QNDialog)

public:
	QNDialog(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~QNDialog();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	CEdit edit_username;
	CEdit edit_password;
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
};
