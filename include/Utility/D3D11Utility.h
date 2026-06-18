#pragma once
#undef max
#include <cassert>
#include <limits>
#include "Math/LinearColor.h"
#include "Rendering/Sampler.h"
#include "Logger.h"
static const unsigned int allMips = std::numeric_limits<unsigned int>::max();
static const LinearColor colorLoad = LinearColor(std::numeric_limits<float>::infinity(), 0.0f, 0.0f, 0.0f);
static const float depthLoad = std::numeric_limits<float>::infinity();
static const unsigned int stencilLoad = std::numeric_limits<unsigned int>::infinity();
static const unsigned char maxRenderTargetCount = 8;
enum class PrimitiveType
{
    TriangleList,
    TriangleStrip,
    LineList,
    QuadList,
    PointList,
    RectList,
    Num,
    NumBits = 3
};
class D3D11Utility
{
public:
    template <typename T>
    static void Release(T* pointer)
    {
        if (pointer)
        {
            pointer->Release();
            pointer = nullptr;
        }
    }
    static unsigned int GetVertexCountForPrimitiveCount(unsigned int primitiveCount, PrimitiveType primitiveType)
    {
        unsigned int factor = (primitiveType == PrimitiveType::TriangleList) ? 3 : (primitiveType == PrimitiveType::LineList) ? 2 : (primitiveType == PrimitiveType::RectList) ? 3 : 1;
        unsigned int offset = (primitiveType == PrimitiveType::TriangleStrip) ? 2 : 0;
        return primitiveCount * factor + offset;
    }
    static D3D11_PRIMITIVE_TOPOLOGY GetD3D11PrimitiveType(PrimitiveType primitiveType)
    {
        switch (primitiveType)
        {
            case PrimitiveType::TriangleList:
                return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            case PrimitiveType::TriangleStrip:
                return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            case PrimitiveType::LineList:
                return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
            case PrimitiveType::PointList:
                return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
            default:
                Logger::GetInstance().Log(Logger::Level::Error, "Unknown primitive type: {}", static_cast<unsigned int>(primitiveType));
        };
        return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    }
};