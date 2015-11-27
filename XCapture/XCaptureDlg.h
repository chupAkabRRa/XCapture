
// XCaptureDlg.h : header file
//

#pragma once

#include "ScreenCapturer.h"
#include <memory>

// CXCaptureDlg dialog
class CXCaptureDlg : public CDialogEx, public ScreenCapturer::Callback
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
    std::shared_ptr<ScreenCapturer> capturer;
    RECT captureRect;
    DWORD dwFps;
    void SaveBmpToFile(BITMAPINFOHEADER& bmif, BYTE *pData, CString fileName);
    static DWORD WINAPI ThreadedTimer(LPVOID lpParam);
    HANDLE hTimerThread;
    DWORD dwTimerThreadId;

public:
    afx_msg void OnDestroy();
    afx_msg void OnBnClickedOk();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
};
