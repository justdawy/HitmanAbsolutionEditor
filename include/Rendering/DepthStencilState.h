#pragma once
#include <d3d11.h>
class DepthStencilState
{
public:
    enum class RHIZBuffer
    {
        FarPlane = 1,
        NearPlane = 0,
        IsInverted = RHIZBuffer::FarPlane < RHIZBuffer::NearPlane,
    };
    enum class CompareFunction
    {
        Less,
        LessEqual,
        Greater,
        GreaterEqual,
        Equal,
        NotEqual,
        Never,
        Always,
        DepthNearOrEqual = (((int)RHIZBuffer::IsInverted != 0) ? GreaterEqual : LessEqual),
        DepthNear = (((int)RHIZBuffer::IsInverted != 0) ? Greater : Less),
        DepthFartherOrEqual = (((int)RHIZBuffer::IsInverted != 0) ? LessEqual : GreaterEqual),
        DepthFarther = (((int)RHIZBuffer::IsInverted != 0) ? Less : Greater),
    };
    enum class StencilOp
    {
        Keep,
        Zero,
        Replace,
        SaturatedIncrement,
        SaturatedDecrement,
        Invert,
        Increment,
        Decrement
    };
    DepthStencilState(
        const bool enableDepthWrite = true,
        const DepthStencilState::CompareFunction depthTest = DepthStencilState::CompareFunction::DepthNearOrEqual,
        const bool enableFrontFaceStencil = false,
        const DepthStencilState::CompareFunction frontFaceStencilTest = DepthStencilState::CompareFunction::Always,
        const DepthStencilState::StencilOp frontFaceStencilFailStencilOp = DepthStencilState::StencilOp::Keep,
        const DepthStencilState::StencilOp frontFaceDepthFailStencilOp = DepthStencilState::StencilOp::Keep,
        const DepthStencilState::StencilOp frontFacePassStencilOp = DepthStencilState::StencilOp::Keep,
        const bool enableBackFaceStencil = false,
        const DepthStencilState::CompareFunction backFaceStencilTest = DepthStencilState::CompareFunction::Always,
        const DepthStencilState::StencilOp backFaceStencilFailStencilOp = DepthStencilState::StencilOp::Keep,
        const DepthStencilState::StencilOp backFaceDepthFailStencilOp = DepthStencilState::StencilOp::Keep,
        const DepthStencilState::StencilOp backFacePassStencilOp = DepthStencilState::StencilOp::Keep,
        const unsigned char stencilReadMask = 0xFF,
        const unsigned char stencilWriteMask = 0xFF
    );
    ~DepthStencilState();
    ID3D11DepthStencilState* GetDepthStencilState() const;
    static D3D11_COMPARISON_FUNC ConvertCompareFunction(const CompareFunction compareFunction);
    static D3D11_STENCIL_OP ConvertStencilOp(const StencilOp stencilOp);
private:
    void CreateResource();
    bool enableDepthWrite;
    D3D11_COMPARISON_FUNC depthTest;
    bool enableFrontFaceStencil;
    D3D11_COMPARISON_FUNC frontFaceStencilTest;
    D3D11_STENCIL_OP frontFaceStencilFailStencilOp;
    D3D11_STENCIL_OP frontFaceDepthFailStencilOp;
    D3D11_STENCIL_OP frontFacePassStencilOp;
    bool enableBackFaceStencil;
    D3D11_COMPARISON_FUNC backFaceStencilTest;
    D3D11_STENCIL_OP backFaceStencilFailStencilOp;
    D3D11_STENCIL_OP backFaceDepthFailStencilOp;
    D3D11_STENCIL_OP backFacePassStencilOp;
    unsigned char stencilReadMask;
    unsigned char stencilWriteMask;
    ID3D11DepthStencilState* depthStencilState;
};