
// XCapture.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CXCaptureApp:
// See XCapture.cpp for the implementation of this class
//

class CXCaptureApp : public CWinApp
{
public:
	CXCaptureApp();

// Overrides
public:
	virtual BOOL InitInstance();
    virtual int ExitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CXCaptureApp theApp;