#pragma once

#include <windows.h>
#include <atlbase.h>
#include <DXGITYPE.h>
#include <DXGI1_2.h>
#include <d3d11.h>
#include <Wincodec.h>
#include <vector>
#include <memory>

class DuplicationOutput
{
public:
    DuplicationOutput(IDXGIAdapter1* pAdapter, ID3D11Device* pDevice, ID3D11DeviceContext *pContext, IDXGIOutput1* pOutput, IDXGIOutputDuplication *pDuplication);
    ~DuplicationOutput();

    HRESULT GetDesc(DXGI_OUTPUT_DESC& desc);
    HRESULT AcquireNextFrame(IDXGISurface1** pDXGISurface);
    HRESULT ReleaseFrame();

private:
    CComPtr<IDXGIAdapter1> m_Adapter;
    CComPtr<ID3D11Device> m_Device;
    CComPtr<ID3D11DeviceContext> m_DeviceContext;
    CComPtr<IDXGIOutput1> m_Output;
    CComPtr<IDXGIOutputDuplication> m_OutputDuplication;
};

class DuplicationManager
{
public:
    DuplicationManager();
    ~DuplicationManager();

    HRESULT CaptureImage(BYTE* pBuf, RECT& rcDest);
    HRESULT Init();

private:
    bool m_bInitialized;
    std::unique_ptr<DuplicationOutput> m_Output;
    CComPtr<IDXGIFactory1> m_spDXGIFactory1;
    ULONG_PTR m_gdiplusToken;
    BYTE* m_pBuf;
    RECT m_rcCurrentOutput;
    CComPtr<IWICImagingFactory> m_spWICFactory;
};
