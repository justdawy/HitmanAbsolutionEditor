#pragma once
#include <d3d11.h>
class SwapChain
{
public:
    enum class PresentMode
    {
        Immediate,
        Mailbox,
        Fifo,
    };
    SwapChain() = default;
    SwapChain(const unsigned int width, const unsigned int height, const PresentMode presentMode, const unsigned int bufferCount);
    ~SwapChain();
    unsigned int GetWidth() const;
    unsigned int GetHeight() const;
    unsigned int GetBufferCount() const;
    DXGI_FORMAT GetFormat() const;
    ID3D11RenderTargetView* GetRenderTargetView() const;
    bool Create();
    bool CreateRenderTargetView();
    bool Resize(unsigned int width, unsigned int height, const bool force = false);
    void Present();
    static UINT GetPresentFlags(const PresentMode presentMode);
    static PresentMode GetSupportedPresentMode(const PresentMode presentMode);
    static bool IsTearingSupported();
private:
    unsigned int width;
    unsigned int height;
    PresentMode presentMode;
    unsigned int bufferCount;
    DXGI_FORMAT format;
    bool windowed;
    IDXGISwapChain* swapChain;
    ID3D11RenderTargetView* renderTargetView;
};