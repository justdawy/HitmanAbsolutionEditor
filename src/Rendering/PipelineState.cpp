#include "Rendering/PipelineState.h"
PipelineState::PipelineState()
{
    renderTargetColorTextures.fill(nullptr);
}
const unsigned int PipelineState::GetWidth() const
{
    if (renderTargetSwapchain)
    {
        return renderTargetSwapchain->GetWidth();
    }
    if (renderTargetColorTextures[0])
    {
        return renderTargetColorTextures[0]->GetWidth();
    }
    if (renderTargetDepthTexture)
    {
        return renderTargetDepthTexture->GetWidth();
    }
    return 0;
}
const unsigned int PipelineState::GetHeight() const
{
    if (renderTargetSwapchain)
    {
        return renderTargetSwapchain->GetHeight();
    }
    if (renderTargetColorTextures[0])
    {
        return renderTargetColorTextures[0]->GetHeight();
    }
    if (renderTargetDepthTexture)
    {
        return renderTargetDepthTexture->GetHeight();
    }
    return 0;
}
const bool PipelineState::IsValid() const
{
    bool hasComputeShader = computeShader ? computeShader->IsCompiled() : false;
    bool hasVertexShader = vertexShader ? vertexShader->IsCompiled() : false;
    bool hasPixelShader = pixelShader ? pixelShader->IsCompiled() : false;
    bool hasRenderTarget = renderTargetColorTextures[0] || renderTargetDepthTexture;
    bool hasBackbuffer = renderTargetSwapchain;
    bool hasGraphicsStates = rasterizerState && blendState && depthStencilState;
    bool isGraphicsPSO = (hasVertexShader || hasPixelShader) && !hasComputeShader;
    bool isComputePSO = hasComputeShader && (!hasVertexShader && !hasPixelShader);
    if (!hasComputeShader && !hasVertexShader && !hasPixelShader)
    {
        return false;
    }
    if (isGraphicsPSO && !hasGraphicsStates)
    {
        return false;
    }
    if (isGraphicsPSO && !hasRenderTarget && !hasBackbuffer)
    {
        if (!hasRenderTarget && !hasBackbuffer)
        {
            return false;
        }
        if (hasRenderTarget && hasBackbuffer)
        {
            return false;
        }
    }
    return true;
}
const bool PipelineState::IsGraphics() const
{
    return (vertexShader != nullptr || pixelShader != nullptr) && !computeShader;
}
const bool PipelineState::IsCompute() const
{
    return computeShader != nullptr && !IsGraphics();
}