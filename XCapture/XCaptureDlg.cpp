
// XCaptureDlg.cpp : implementation file
//

#include "stdafx.h"
#include "XCapture.h"
#include "XCaptureDlg.h"
#include "afxdialogex.h"
#include "ScreenCapturerMagnifier.h"
#include "ScreenCapturerDuplication.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DWORD WINAPI CXCaptureDlg::CaptureThreadFunc(LPVOID lpParam)
{
    IXCaptureThread* captureThread = (IXCaptureThread*)lpParam;

    while ((WaitForSingleObjectEx(captureThread->hTerminateThreadEvent, 0, FALSE) == WAIT_TIMEOUT))
    {
        captureThread->capturer->Capture(captureThread->captureRect);
    }

    return 0;
}

// CXCaptureDlg dialog

CXCaptureDlg::CXCaptureDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CXCaptureDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CXCaptureDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CXCaptureDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_WM_DESTROY()
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_BUTTON_EXIT, &CXCaptureDlg::OnBnClickedButtonExit)
END_MESSAGE_MAP()


// CXCaptureDlg message handlers

BOOL CXCaptureDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
    capturer = NULL;
    dwFps = 0;
    hCaptureThread = NULL;

    // Get scr resolution
    RECT rect;
    HWND hDesktop = ::GetDesktopWindow();
    ::GetWindowRect(hDesktop, &rect);

    // Set capture reginon
    captureRect.top = 0;
    captureRect.left = 0;
    captureRect.right = 1024;
    captureRect.bottom = 768;

    hInstance = AfxGetInstanceHandle();

    capturer = std::make_unique<ScreenCapturerMagnifier>();
    capturer->Start(this);

    // Show FPS in caption
    SetWindowText(L"FPS: 0");

    hTerminateThreadEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    hCaptureThread = CreateThread(NULL, 0, CaptureThreadFunc, (IXCaptureThread*)this, 0, &dwCaptureThreadId);
    SetTimer(100, 1000, NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CXCaptureDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CXCaptureDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CXCaptureDlg::OnCaptureComplete(BYTE* pData, BITMAPINFOHEADER* bmif)
{
    BITMAPINFO bmi;
    HBITMAP hbmp;
    CDC *pDC = GetDC();

    // The data bit is in top->bottom order, so we convert it to bottom->top order
    LONG lineSize = bmif->biWidth * bmif->biBitCount / 8;
    BYTE* pLineData = new BYTE[lineSize];
    BYTE* pStart;
    BYTE* pEnd;
    LONG lineStart = 0;
    LONG lineEnd = bmif->biHeight - 1;
    while (lineStart < lineEnd)
    {
        // Get the address of the swap line
        pStart = pData + (lineStart * lineSize);
        pEnd = pData + (lineEnd * lineSize);
        // Swap the top with the bottom
        memcpy(pLineData, pStart, lineSize);
        memcpy(pStart, pEnd, lineSize);
        memcpy(pEnd, pLineData, lineSize);

        // Adjust the line index
        lineStart++;
        lineEnd--;
    }
    delete[] pLineData;

    bmi.bmiHeader = *bmif;
    hbmp = CreateDIBitmap(*pDC, bmif, CBM_INIT, pData, &bmi, DIB_RGB_COLORS);

    HDC hdcMem = CreateCompatibleDC(*pDC);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmp);

    BitBlt(*pDC, 0, 0, bmi.bmiHeader.biWidth, bmi.bmiHeader.biHeight, hdcMem, 0, 0, SRCCOPY);
    SelectObject(hdcMem, hbmOld);
    DeleteDC(hdcMem);
    DeleteObject(hbmp);
    ReleaseDC(pDC);
    
    dwFps++;
}

void CXCaptureDlg::OnDestroy()
{
    __super::OnDestroy();
}

void CXCaptureDlg::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == 100) {
        WCHAR str[10] = {0};
        swprintf_s(str, 10, L"FPS: %d", dwFps);
        SetWindowText(str);
        dwFps = 0;
    }

    __super::OnTimer(nIDEvent);
}

void CXCaptureDlg::StopCaptureThread()
{
    // Terminate timer thread
    if (hCaptureThread) {
        SetEvent(hTerminateThreadEvent);
        WaitForSingleObject(hCaptureThread, INFINITE);
        CloseHandle(hCaptureThread);
        hCaptureThread = NULL;
    }

    CloseHandle(hTerminateThreadEvent);
    hTerminateThreadEvent = NULL;
}

void CXCaptureDlg::OnCancel()
{
    StopCaptureThread();
    __super::OnCancel();
}

void CXCaptureDlg::OnOK()
{
    StopCaptureThread();
    __super::OnOK();
}

void CXCaptureDlg::OnBnClickedButtonExit()
{
    StopCaptureThread();
    DestroyWindow();
}
