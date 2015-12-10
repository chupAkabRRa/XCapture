#include "stdafx.h"
#include "ScreenCapturerDuplication.h"

ScreenCapturerDuplication::ScreenCapturerDuplication()
    : m_pBuf(nullptr)
{
    pDuplicationManager = std::make_unique<DuplicationManager>();
}

ScreenCapturerDuplication::~ScreenCapturerDuplication()
{
    if (m_pBuf) {
        delete[] m_pBuf;
        m_pBuf = nullptr;
    }
}

void ScreenCapturerDuplication::Start(Callback* callback)
{
    m_Callback = callback;

    pDuplicationManager->Init();
}

void ScreenCapturerDuplication::Capture(RECT srcRect)
{
    HRESULT hr;
    DWORD dwDstWidth = srcRect.right - srcRect.left;
    DWORD dwDstHeight = srcRect.bottom - srcRect.top;
    DWORD dwSize = dwDstWidth * dwDstHeight * 4;

    if (m_pBuf) {
        delete[] m_pBuf;
        m_pBuf = nullptr;
    }

    m_pBuf = new BYTE[dwSize];

    hr = pDuplicationManager->CaptureImage(m_pBuf, srcRect);

    if (!FAILED(hr)) {
        m_bmif.biSize = sizeof(BITMAPINFOHEADER);
        m_bmif.biHeight = dwDstHeight;
        m_bmif.biWidth = dwDstWidth;
        m_bmif.biSizeImage = dwSize;
        m_bmif.biPlanes = 1;
        m_bmif.biBitCount = (WORD)(m_bmif.biSizeImage / m_bmif.biHeight / m_bmif.biWidth * 8);
        m_bmif.biCompression = BI_RGB;

        m_Callback->OnCaptureComplete(m_pBuf, &m_bmif);
    }
}

