#pragma once

class ScreenCapturer
{
public:
    class Callback
    {
    public:
        virtual void OnCaptureComplete(BYTE* data, BITMAPINFOHEADER* bmif) = 0;

    protected:
        virtual ~Callback() {}
    };

    virtual ~ScreenCapturer() {}

    // Call to start capturing
    virtual bool Start(Callback* callback) = 0;

    virtual void Capture(RECT srcRect) = 0;

    virtual void SetExcludedWindow(HWND hWindow) {}
};
