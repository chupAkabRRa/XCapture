#pragma once

#include <Windows.h>
#include "ScreenCapturer.h"
#include "DuplicationWrapper.h"

class ScreenCapturerDuplication : public ScreenCapturer
{
public:
    ScreenCapturerDuplication();
    ~ScreenCapturerDuplication();

    // ScreenCapturer interface
    void Start(Callback* callback) override;
    void Capture(RECT srcRect) override;

private:
    Callback* m_Callback;
    std::unique_ptr<DuplicationManager> pDuplicationManager;
    BYTE *m_pBuf;
    BITMAPINFOHEADER m_bmif;
};