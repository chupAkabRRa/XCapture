
// XCaptureDlg.h : header file
//

#pragma once

#include "ScreenCapturer.h"
#include <memory>

class IXCaptureThread
{
public:
    std::unique_ptr<ScreenCapturer> capturer;
    RECT captureRect;
    DWORD dwCaptureThreadId;
    HANDLE hTerminateThreadEvent;
};

// CXCaptureDlg dialog
class CXCaptureDlg 
    : public CDialogEx
    , public ScreenCapturer::Callback
    , public IXCaptureThread
{
// Construction
public:
	CXCaptureDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_XCAPTURE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

    // Callback interface
    void OnCaptureComplete(BYTE* pData, BITMAPINFOHEADER* bmif) override;

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

    HINSTANCE hInstance;
    
    DWORD dwFps;
    static DWORD WINAPI CaptureThreadFunc(LPVOID lpParam);
    void StopCaptureThread();
    HANDLE hCaptureThread;

public:
    afx_msg void OnDestroy();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnBnClickedButtonExit();
    virtual void OnCancel();
    virtual void OnOK();
};
