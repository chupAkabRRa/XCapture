#pragma once


// CHostDlg dialog

class CHostDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CHostDlg)

public:
	CHostDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CHostDlg();

// Dialog Data
	enum { IDD = IDD_HOSTDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
    virtual BOOL Create();
    virtual BOOL OnInitDialog();

protected:
    UINT nID;
    CWnd* parentWnd;
};
