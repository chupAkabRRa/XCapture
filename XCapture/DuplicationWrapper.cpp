#include "stdafx.h"
#include "DuplicationWrapper.h"

#include <gdiplus.h>

#pragma comment(lib, "D3D11.lib")
#pragma comment(lib, "DXGI.lib")

//
// Duplication wrapper
//

DuplicationOutput::DuplicationOutput(IDXGIAdapter1* pAdapter, ID3D11Device* pDevice, ID3D11DeviceContext *pContext, IDXGIOutput1* pOutput, IDXGIOutputDuplication *pDuplication)
    : m_Adapter(pAdapter)
    , m_Device(pDevice)
    , m_DeviceContext(pContext)
    , m_Output(pOutput)
    , m_OutputDuplication(pDuplication)
{}

DuplicationOutput::~DuplicationOutput()
{
    m_OutputDuplication->ReleaseFrame();
}

HRESULT DuplicationOutput::GetDesc(DXGI_OUTPUT_DESC& desc)
{
    m_Output->GetDesc(&desc);
    return S_OK;
}

HRESULT DuplicationOutput::ReleaseFrame()
{
    m_OutputDuplication->ReleaseFrame();
    return S_OK;
}

HRESULT DuplicationOutput::AcquireNextFrame(IDXGISurface1** pDXGISurface)
{
    DXGI_OUTDUPL_FRAME_INFO fi;
    CComPtr<IDXGIResource> spDXGIResource;
    HRESULT hr = m_OutputDuplication->AcquireNextFrame(20, &fi, &spDXGIResource);
    if (FAILED(hr))
    {
        return hr;
    }

    CComQIPtr<ID3D11Texture2D> spTextureResource = spDXGIResource;

    D3D11_TEXTURE2D_DESC desc;
    spTextureResource->GetDesc(&desc);

    D3D11_TEXTURE2D_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(texDesc));
    texDesc.Width = desc.Width;
    texDesc.Height = desc.Height;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Usage = D3D11_USAGE_STAGING;
    texDesc.Format = desc.Format;
    texDesc.BindFlags = 0;
    texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    texDesc.MiscFlags = 0;

    CComPtr<ID3D11Texture2D> spD3D11Texture2D = NULL;
    hr = m_Device->CreateTexture2D(&texDesc, NULL, &spD3D11Texture2D);
    if (FAILED(hr))
        return hr;

    m_DeviceContext->CopyResource(spD3D11Texture2D, spTextureResource);

    CComQIPtr<IDXGISurface1> spDXGISurface = spD3D11Texture2D;

    *pDXGISurface = spDXGISurface.Detach();

    return hr;
}

//
// Manager
//

DuplicationManager::DuplicationManager()
    : m_bInitialized(false)
    , m_pBuf(NULL)
{
    SetRect(&m_rcCurrentOutput, 0, 0, 0, 0);
}

DuplicationManager::~DuplicationManager()
{
    Gdiplus::GdiplusShutdown(m_gdiplusToken);

    if (m_pBuf)
    {
        delete[] m_pBuf;
        m_pBuf = NULL;
    }
}

HRESULT DuplicationManager::Init()
{
    if (m_bInitialized) {
        return S_OK;
    }

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

    HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&m_spDXGIFactory1));
    if (FAILED(hr))
    {
        return hr;
    }

    // Getting all adapters
    std::vector<CComPtr<IDXGIAdapter1>> vAdapters;

    CComPtr<IDXGIAdapter1> spAdapter;
    for (int i = 0; m_spDXGIFactory1->EnumAdapters1(i, &spAdapter) != DXGI_ERROR_NOT_FOUND; i++)
    {
        vAdapters.push_back(spAdapter);
        spAdapter.Release();
    }

    // Iterating over all adapters to get all outputs
    for (auto& AdapterIter : vAdapters)
    {
        std::vector<CComPtr<IDXGIOutput>> vOutputs;

        CComPtr<IDXGIOutput> spDXGIOutput;
        for (int i = 0; AdapterIter->EnumOutputs(i, &spDXGIOutput) != DXGI_ERROR_NOT_FOUND; i++)
        {
            DXGI_OUTPUT_DESC outputDesc;
            spDXGIOutput->GetDesc(&outputDesc);

            if (outputDesc.AttachedToDesktop)
            {
                vOutputs.push_back(spDXGIOutput);
            }

            spDXGIOutput.Release();
        }

        if (vOutputs.size() == 0)
            continue;

        // Creating device for each adapter that has the output
        CComPtr<ID3D11Device> spD3D11Device;
        CComPtr<ID3D11DeviceContext> spD3D11DeviceContext;
        D3D_FEATURE_LEVEL fl = D3D_FEATURE_LEVEL_9_1;
        hr = D3D11CreateDevice(AdapterIter, D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, NULL, 0, D3D11_SDK_VERSION, &spD3D11Device, &fl, &spD3D11DeviceContext);
        if (FAILED(hr))
        {
            return hr;
        }

        for (auto& OutputIter : vOutputs)
        {
            CComQIPtr<IDXGIOutput1> spDXGIOutput1 = OutputIter;
            CComQIPtr<IDXGIDevice1> spDXGIDevice = spD3D11Device;
            if (!spDXGIOutput1 || !spDXGIDevice)
                continue;

            CComPtr<IDXGIOutputDuplication> spDXGIOutputDuplication;
            hr = spDXGIOutput1->DuplicateOutput(spDXGIDevice, &spDXGIOutputDuplication);
            if (FAILED(hr))
                continue;

            // Only work with primary monitor now
            DXGI_OUTPUT_DESC outdesc;
            spDXGIOutput1->GetDesc(&outdesc);

            MONITORINFO mi;
            mi.cbSize = sizeof(MONITORINFO);
            GetMonitorInfo(outdesc.Monitor, &mi);
            if (mi.dwFlags & MONITORINFOF_PRIMARY) {
                m_Output = std::make_unique<DuplicationOutput>(
                    AdapterIter,
                    spD3D11Device,
                    spD3D11DeviceContext,
                    spDXGIOutput1,
                    spDXGIOutputDuplication);
            }
        }
    }

    hr = m_spWICFactory.CoCreateInstance(CLSID_WICImagingFactory);
    if (FAILED(hr))
    {
        return hr;
    }

    m_bInitialized = true;

    return S_OK;
}

HRESULT DuplicationManager::CaptureImage(BYTE* pBits, RECT& rcDest)
{
    HRESULT hr = S_OK;

    DWORD dwDstWidth = rcDest.right - rcDest.left;
    DWORD dwDstHeight = rcDest.bottom - rcDest.top;

    RECT rcOutput;
    DXGI_OUTPUT_DESC outDesc;
    hr = m_Output->GetDesc(outDesc);
    if (FAILED(hr)) {
        return hr;
    }

    rcOutput = outDesc.DesktopCoordinates;

    DWORD dwOutputWidth = rcOutput.right - rcOutput.left;
    DWORD dwOutputHeight = rcOutput.bottom - rcOutput.top;

    if (!m_pBuf || !EqualRect(&m_rcCurrentOutput, &rcOutput))
    {
        DWORD dwBufSize = dwOutputWidth*dwOutputHeight * 4;

        if (m_pBuf)
        {
            delete[] m_pBuf;
            m_pBuf = NULL;
        }

        m_pBuf = new BYTE[dwBufSize];

        CopyRect(&m_rcCurrentOutput, &rcOutput);
    }

    CComPtr<IDXGISurface1> spDXGISurface1;
    hr = m_Output->AcquireNextFrame(&spDXGISurface1);
    if (FAILED(hr)) {
        return hr;
    }

    DXGI_MAPPED_RECT map;
    spDXGISurface1->Map(&map, DXGI_MAP_READ);

    RECT rcDesktop = outDesc.DesktopCoordinates;
    DWORD dwWidth = rcDesktop.right - rcDesktop.left;
    DWORD dwHeight = rcDesktop.bottom - rcDesktop.top;

    OffsetRect(&rcDesktop, -rcOutput.left, -rcOutput.top);

    DWORD dwMapPitchPixels = map.Pitch / 4;

    switch (outDesc.Rotation)
    {
    case DXGI_MODE_ROTATION_IDENTITY:
    {
        // Just copying
        DWORD dwStripe = dwWidth * 4;
        for (unsigned int i = 0; i<dwHeight; i++)
        {
            memcpy_s(m_pBuf + (rcDesktop.left + (i + rcDesktop.top)*dwOutputWidth) * 4, dwStripe, map.pBits + i*map.Pitch, dwStripe);
        }
    }
    break;
    case DXGI_MODE_ROTATION_ROTATE90:
    {
        // Rotating at 90 degrees
        DWORD* pSrc = (DWORD*)map.pBits;
        DWORD* pDst = (DWORD*)m_pBuf;
        for (unsigned int j = 0; j<dwHeight; j++)
        {
            for (unsigned int i = 0; i<dwWidth; i++)
            {
                *(pDst + (rcDesktop.left + (j + rcDesktop.top)*dwOutputWidth) + i) = *(pSrc + j + dwMapPitchPixels*(dwWidth - i - 1));
            }
        }
    }
    break;
    case DXGI_MODE_ROTATION_ROTATE180:
    {
        // Rotating at 180 degrees
        DWORD* pSrc = (DWORD*)map.pBits;
        DWORD* pDst = (DWORD*)m_pBuf;
        for (unsigned int j = 0; j<dwHeight; j++)
        {
            for (unsigned int i = 0; i<dwWidth; i++)
            {
                *(pDst + (rcDesktop.left + (j + rcDesktop.top)*dwOutputWidth) + i) = *(pSrc + (dwWidth - i - 1) + dwMapPitchPixels*(dwHeight - j - 1));
            }
        }
    }
    break;
    case DXGI_MODE_ROTATION_ROTATE270:
    {
        // Rotating at 270 degrees
        DWORD* pSrc = (DWORD*)map.pBits;
        DWORD* pDst = (DWORD*)m_pBuf;
        for (unsigned int j = 0; j<dwHeight; j++)
        {
            for (unsigned int i = 0; i<dwWidth; i++)
            {
                *(pDst + (rcDesktop.left + (j + rcDesktop.top)*dwOutputWidth) + i) = *(pSrc + (dwHeight - j - 1) + dwMapPitchPixels*i);
            }
        }
    }
    break;
    }

    spDXGISurface1->Unmap();
    m_Output->ReleaseFrame();

    // m_pBuf now filled with image
    CComPtr<IWICBitmap> spBitmap = NULL;
    hr = m_spWICFactory->CreateBitmapFromMemory(dwOutputWidth, dwOutputHeight, GUID_WICPixelFormat32bppBGRA, dwOutputWidth * 4, dwOutputWidth*dwOutputHeight * 4, (BYTE*)m_pBuf, &spBitmap);
    if (FAILED(hr)) {
        return hr;
    }

    WICRect rect;
    rect.X = 0;
    rect.Y = 0;
    rect.Width = dwDstWidth;
    rect.Height = dwDstHeight;
    spBitmap->CopyPixels(&rect, dwDstWidth * 4, dwDstWidth * dwDstHeight * 4, pBits);

    return hr;
}