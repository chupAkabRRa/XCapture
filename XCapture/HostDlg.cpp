// HostDlg.cpp : implementation file
//

#include "stdafx.h"
#include "XCapture.h"
#include "HostDlg.h"
#include "afxdialogex.h"


// CHostDlg dialog

IMPLEMENT_DYNAMIC(CHostDlg, CDialogEx)

CHostDlg::CHostDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CHostDlg::IDD, pParent)
    , nID(CHostDlg::IDD)
    , parentWnd(pParent)
{

}

CHostDlg::~CHostDlg()
{
}

void CHostDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CHostDlg, CDialogEx)
END_MESSAGE_MAP()


// CHostDlg message handlers
BOOL CHostDlg::Create()
{
    CDialogEx::Create(nID, parentWnd);
    return TRUE;
}

BOOL CHostDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    RECT rect;
    HWND hDesktop = ::GetDesktopWindow();
    ::GetWindowRect(hDesktop, &rect);

    SetWindowPos(NULL, 0, 0, rect.right, rect.bottom, SWP_HIDEWINDOW);

    SetWindowLong(this->GetSafeHwnd(), GWL_EXSTYLE, GetWindowLong(this->GetSafeHwnd(), GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(0, 255, LWA_ALPHA);

    return TRUE;
}
