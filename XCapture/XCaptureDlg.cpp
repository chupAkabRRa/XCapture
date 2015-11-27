
// XCaptureDlg.cpp : implementation file
//

#include "stdafx.h"
#include "XCapture.h"
#include "XCaptureDlg.h"
#include "afxdialogex.h"
#include "ScreenCapturerMagnifier.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DWORD WINAPI CXCaptureDlg::ThreadedTimer(LPVOID lpParam)
{
    CXCaptureDlg* captureDlg = (CXCaptureDlg*)lpParam;

    while (TRUE)
    {
        captureDlg->capturer->Capture(captureDlg->captureRect);
    }
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
    ON_BN_CLICKED(IDOK, &CXCaptureDlg::OnBnClickedOk)
    ON_WM_ERASEBKGND()
    ON_WM_TIMER()
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
    hTimerThread = NULL;

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

    capturer = std::make_shared<ScreenCapturerMagnifier>();
    capturer->Start(this);

    hTimerThread = CreateThread(NULL, 0, ThreadedTimer, this, 0, &dwTimerThreadId);
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
    //TRACE(L"PROCESSED!");
    //SaveBmpToFile(*bmif, pData, L"D:\\test.bmp");
    BITMAPINFO bmi;
    HBITMAP hbmp;
    CDC *pDC = GetDC();

    bmi.bmiHeader = *bmif;
    hbmp = CreateDIBitmap(*pDC, bmif, CBM_INIT, pData, &bmi, DIB_RGB_COLORS);

    HDC hdcMem = CreateCompatibleDC(*pDC);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmp);

    BitBlt(*pDC, 0, 0, bmi.bmiHeader.biWidth, bmi.bmiHeader.biHeight, hdcMem, 0, 0, SRCCOPY);
    SelectObject(hdcMem, hbmOld);
    DeleteDC(hdcMem);
    ReleaseDC(pDC);
    
    dwFps++;
}

void CXCaptureDlg::OnDestroy()
{
    __super::OnDestroy();

    // TODO: Add your message handler code here
    if (hTimerThread) {
        CloseHandle(hTimerThread);
    }
}


void CXCaptureDlg::OnBnClickedOk()
{
    // TODO: Add your control notification handler code here
    // Get the screen rectangle
    capturer->SetExcludedWindow(this->GetSafeHwnd());
}

void CXCaptureDlg::SaveBmpToFile(BITMAPINFOHEADER& bmif, BYTE *pData, CString fileName)
{
    // File open
    CFile pFile;
    if (!pFile.Open((LPCTSTR)fileName, CFile::modeCreate | CFile::modeWrite))
    {
        return;
    }

    // Setup the bitmap file header
    BITMAPFILEHEADER bmfh;
    LONG offBits = sizeof(BITMAPFILEHEADER) + bmif.biSize;
    bmfh.bfType = 0x4d42; // "BM"
    bmfh.bfOffBits = offBits;
    bmfh.bfSize = offBits + bmif.biSizeImage;
    bmfh.bfReserved1 = 0;
    bmfh.bfReserved2 = 0;

    //Write data to file
    pFile.Write(&bmfh, sizeof(BITMAPFILEHEADER)); // bitmap file header
    pFile.Write(&bmif, sizeof(BITMAPINFOHEADER)); // bitmap info header
    pFile.Write(pData, bmif.biSizeImage); // converted bitmap data

    // File close
    pFile.Close();
}


BOOL CXCaptureDlg::OnEraseBkgnd(CDC* pDC)
{
    // TODO: Add your message handler code here and/or call default

    return __super::OnEraseBkgnd(pDC);
}


void CXCaptureDlg::OnTimer(UINT_PTR nIDEvent)
{
    // TODO: Add your message handler code here and/or call default
    if (nIDEvent == 100) {
        CWnd *label = GetDlgItem(IDC_FPS);
        WCHAR str[5];
        swprintf_s(str, 3, L"%d", dwFps);
        label->SetWindowTextW(str);
        UpdateData(FALSE);
        dwFps = 0;
    }

    __super::OnTimer(nIDEvent);
}
