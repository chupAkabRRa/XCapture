#include "stdafx.h"
#include "ScreenCapturerDuplication.h"

ScreenCapturerDuplication::ScreenCapturerDuplication()
    : m_pBuf(nullptr)
{
    pDuplicationManager = std::make_unique<DuplicationManager>();
}

ScreenCapturerDuplication::~ScreenCapturerDuplication()
{

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

        // The data bit is in top->bottom order, so we convert it to bottom->top order
        LONG lineSize = m_bmif.biWidth * m_bmif.biBitCount / 8;
        BYTE* pLineData = new BYTE[lineSize];
        BYTE* pStart;
        BYTE* pEnd;
        LONG lineStart = 0;
        LONG lineEnd = m_bmif.biHeight - 1;
        while (lineStart < lineEnd)
        {
            // Get the address of the swap line
            pStart = m_pBuf + (lineStart * lineSize);
            pEnd = m_pBuf + (lineEnd * lineSize);
            // Swap the top with the bottom
            memcpy(pLineData, pStart, lineSize);
            memcpy(pStart, pEnd, lineSize);
            memcpy(pEnd, pLineData, lineSize);

            // Adjust the line index
            lineStart++;
            lineEnd--;
        }
        delete[] pLineData;

        m_Callback->OnCaptureComplete(m_pBuf, &m_bmif);
    }
}

