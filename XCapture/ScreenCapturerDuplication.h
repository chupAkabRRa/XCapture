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
    bool Start(Callback* callback) override;
    void Capture(RECT srcRect) override;

private:
    // Callback to call when image is received from Magnification API
    Callback* m_Callback;

    // Manages all the stuff related to Duplication API
    std::unique_ptr<DuplicationManager> pDuplicationManager;

    // Data to pass to renderer callback at the end
    BYTE* m_pBuf;
    BITMAPINFOHEADER m_bmif;
};