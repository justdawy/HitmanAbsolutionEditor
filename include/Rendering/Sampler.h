#pragma once
#include <d3d11.h>
class Sampler
{
public:
    enum class Filter
    {
        MinMagMipPoint = 0,
        MinMagPointMipLinear = 1,
        MinPointMagLinearMipPoint = 4,
        MinPointMagMipLinear = 5,
        MinLinearMagMipPoint = 16,
        MinLinearMagPointMipLinear = 17,
        MinMagLinearMipPoint = 20,
        MinMagMipLinear = 21,
        Anisotropic = 85,
        ComparisonMinMagMipPoint = 128,
        ComparisonMinMagPointMipLinear = 129,
        ComparisonMinPointMagLinearMipPoint = 132,
        ComparisonMinPointMagMipLinear = 133,
        ComparisonMinLinearMagMipPoint = 144,
        ComparisonMinLinearMagPointMipLinear = 145,
        ComparisonMinMagLinearMipPoint = 148,
        ComparisonMinMagMipLinear = 149,
        ComparisonAnisotropic = 213,
        MinimumMinMagMipPoint = 256,
        MinimumMinMagPointMipLinear = 257,
        MinimumMinPointMagLinearMipPoint = 260,
        MinimumMinPointMagMipLinear = 261,
        MinimumMinLinearMagMipPoint = 272,
        MinimumMinLinearMagPointMipLinear = 273,
        MinimumMinMagLinearMipPoint = 276,
        MinimumMinMagMipLinear = 277,
        MinimumAnisotropic = 341,
        MaximumMinMagMipPoint = 384,
        MaximumMinMagPointMipLinear = 385,
        MaximumMinPointMagLinearMipPoint = 388,
        MaximumMinPointMagMipLinear = 389,
        MaximumMinLinearMagMipPoint = 400,
        MaximumMinLinearMagPointMipLinear = 401,
        MaximumMinMagLinearMipPoint = 404,
        MaximumMinMagMipLinear = 405,
        MaximumAnisotropic = 469
    };
    enum class AddressMode
    {
        Wrap,
        Clamp,
        Mirror,
        Border
    };
    enum class CompareFunction
    {
        Never,
        Less
    };
    Sampler(
        const Filter filter = Filter::MinMagMipLinear,
        const AddressMode addressU = AddressMode::Clamp,
        const AddressMode addressV = AddressMode::Clamp,
        const AddressMode addressW = AddressMode::Clamp,
        const float mipLodBias = 0.f,
        const unsigned int maxAnisotropy = D3D11_MAX_MAXANISOTROPY,
        const unsigned int borderColor = 0,
        const CompareFunction compareFunction = CompareFunction::Never
    );
    ~Sampler();
    ID3D11SamplerState* GetSamplerState() const;
    static D3D11_TEXTURE_ADDRESS_MODE ConvertAddressMode(const AddressMode addressMode);
    static D3D11_COMPARISON_FUNC ConvertCompareFunction(const CompareFunction compareFunction);
private:
    void CreateResource();
    Filter filter;
    D3D11_TEXTURE_ADDRESS_MODE addressU;
    D3D11_TEXTURE_ADDRESS_MODE addressV;
    D3D11_TEXTURE_ADDRESS_MODE addressW;
    float mipLodBias;
    unsigned int maxAnisotropy;
    unsigned int borderColor;
    bool isComparisonEnabled;
    D3D11_COMPARISON_FUNC comparisonFunction;
    ID3D11SamplerState* samplerState = nullptr;
};