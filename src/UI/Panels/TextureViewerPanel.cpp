#include <DDSTextureLoader.h>
#include "Utility/UI.h"
#include "UI/Panels/TextureViewerPanel.h"
#include "Utility/D3D11Utility.h"
#include "Utility/Math.h"
#include "Editor.h"
TextureViewerPanel::TextureViewerPanel(const char* name, const char* icon, std::shared_ptr<Texture> textureResource, float* sliderValue) : BasePanel(name, icon)
{
    this->textureResource = textureResource;
    texture = nullptr;
    textureView = nullptr;
    zoom = 1.f;
    zoomMode = ZoomMode::Fit;
    this->sliderValue = sliderValue;
    isRedChannelEnabled = true;
    isGreenChannelEnabled = true;
    isBlueChannelEnabled = true;
    isAlphaChannelEnabled = true;
    pixelShader = std::make_shared<Shader>();
    channelControlConstantBufferGpu = std::make_shared<ConstantBuffer>();
}
TextureViewerPanel::~TextureViewerPanel()
{
	D3D11Utility::Release(textureView);
	D3D11Utility::Release(texture);
}
void TextureViewerPanel::Render()
{
    if (!Begin())
    {
        return;
    }
    if (!textureView)
    {
        ImGui::SetCursorPos(ImVec2(ImGui::GetContentRegionAvail().x / 2, ImGui::GetContentRegionAvail().y / 2));
        ImGui::Text("Loading texture...");
        End();
        return;
    }
    RenderTexture();
    bool isTextureHovered = ImGui::IsItemHovered();
    if (isTextureHovered && ImGui::IsMouseDragging(ImGuiPopupFlags_MouseButtonLeft))
    {
        if (ImGui::IsMouseDragging(ImGuiPopupFlags_MouseButtonLeft))
        {
            const ImVec2 mouseDragDelta = ImGui::GetMouseDragDelta(ImGuiPopupFlags_MouseButtonLeft);
            ImGui::SetScrollX(ImGui::GetScrollX() - mouseDragDelta.x);
            ImGui::SetScrollY(ImGui::GetScrollY() - mouseDragDelta.y);
            ImGui::ResetMouseDragDelta(0);
        }
    }
    if (ImGui::IsWindowHovered())
    {
        ImGuiIO& io = ImGui::GetIO();
        if (io.MouseWheel > 0)
        {
            ZoomIn();
        }
        else if (io.MouseWheel < 0)
        {
            ZoomOut();
        }
    }
    UpdateZoomSliderValue();
    End();
}
const TextureViewerPanel::ZoomMode TextureViewerPanel::GetZoomMode() const
{
    return zoomMode;
}
void TextureViewerPanel::SetZoomMode(const ZoomMode zoomMode)
{
    this->zoomMode = zoomMode;
}
void TextureViewerPanel::SetTextureWidth(const unsigned int textureWidth)
{
    this->textureWidth = textureWidth;
}
void TextureViewerPanel::SetTextureHeight(const unsigned int textureHeight)
{
    this->textureHeight = textureHeight;
}
void TextureViewerPanel::SetTexture(ID3D11Resource* texture)
{
    this->texture = texture;
}
void TextureViewerPanel::SetTextureView(ID3D11ShaderResourceView* textureView)
{
    this->textureView = textureView;
}
void TextureViewerPanel::OnResourceLoaded()
{
    pixelShader->Compile(Shader::Type::Pixel, "assets/shaders/ChannelControl_PS.hlsl");
    channelControlConstantBufferGpu->Create<ChannelControlConstantBuffer>(1);
    if (textureResource)
    {
        unsigned short width;
        unsigned short height;
        DirectX::Blob blob;
        textureResource->Deserialize();
        textureResource->ConvertTextureToDDS(blob, width, height);
        DirectX::CreateDDSTextureFromMemory(Editor::GetInstance().GetDirectXRenderer()->GetD3D11Device(), static_cast<unsigned char*>(blob.GetBufferPointer()), blob.GetBufferSize(), &texture, &textureView);
        textureWidth = static_cast<unsigned int>(width);
        textureHeight = static_cast<unsigned int>(height);
    }
}
void TextureViewerPanel::OnZoomSliderValueChange()
{
    double octaves = log2(maxZoom / minZoom);
    double zoomValue = pow(2.0, *sliderValue * octaves) * minZoom;
    SetCustomZoomLevel((float)zoomValue);
}
void TextureViewerPanel::SetIsRedChannelEnabled(const bool isRedChannelEnabled)
{
    this->isRedChannelEnabled = isRedChannelEnabled;
}
void TextureViewerPanel::SetIsGreenChannelEnabled(const bool isGreenChannelEnabled)
{
    this->isGreenChannelEnabled = isGreenChannelEnabled;
}
void TextureViewerPanel::SetIsBlueChannelEnabled(const bool isBlueChannelEnabled)
{
    this->isBlueChannelEnabled = isBlueChannelEnabled;
}
void TextureViewerPanel::SetIsAlphaChannelEnabled(const bool isAlphaChannelEnabled)
{
    this->isAlphaChannelEnabled = isAlphaChannelEnabled;
}
double TextureViewerPanel::CalculateDisplayedZoomLevel() const
{
    if (zoomMode == ZoomMode::Custom)
    {
        return zoom;
    }
    unsigned int displayWidth, displayHeight;
    CalculateTextureDimensions(displayWidth, displayHeight);
    if (textureHeight != 0)
    {
        return static_cast<double>(displayHeight) / textureHeight;
    }
    else if (textureWidth != 0)
    {
        return static_cast<double>(displayWidth) / textureWidth;
    }
    return 0;
}
void TextureViewerPanel::OnSelectedDDSTexture(ID3D11ShaderResourceView* textureView, const unsigned int textureWidth, const unsigned int textureHeight)
{
    this->textureView = textureView;
    this->textureWidth = textureWidth;
    this->textureHeight = textureHeight;
}
void TextureViewerPanel::RenderTexture()
{
    viewportSize = ImGui::GetContentRegionAvail();
    unsigned int displayWidth;
    unsigned int displayHeight;
    CalculateTextureDimensions(displayWidth, displayHeight);
    const float textureWidth = static_cast<float>(displayWidth);
    const float textureHeight = static_cast<float>(displayHeight);
    ImGui::SetCursorPos(ImVec2((viewportSize.x - textureWidth) / 2, (viewportSize.y - textureHeight) / 2));
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddCallback(SetRenderState, this);
    ImGui::Image(textureView, ImVec2(textureWidth, textureHeight));
    drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
}
void TextureViewerPanel::SetRenderState(const ImDrawList* drawList, const ImDrawCmd* cmd)
{
    TextureViewerPanel* textureViewerPanel = static_cast<TextureViewerPanel*>(cmd->UserCallbackData);
    ID3D11PixelShader* pixelShader = static_cast<ID3D11PixelShader*>(textureViewerPanel->pixelShader->GetResource());
    ID3D11Buffer* channelControlConstantBufferGpu2 = textureViewerPanel->channelControlConstantBufferGpu->GetBuffer();
    textureViewerPanel->channelControlConstantBufferCpu.isRedChannelEnabled = textureViewerPanel->isRedChannelEnabled;
    textureViewerPanel->channelControlConstantBufferCpu.isGreenChannelEnabled = textureViewerPanel->isGreenChannelEnabled;
    textureViewerPanel->channelControlConstantBufferCpu.isBlueChannelEnabled = textureViewerPanel->isBlueChannelEnabled;
    textureViewerPanel->channelControlConstantBufferCpu.isAlphaChannelEnabled = textureViewerPanel->isAlphaChannelEnabled;
    textureViewerPanel->channelControlConstantBufferGpu->Update(&textureViewerPanel->channelControlConstantBufferCpu);
    Editor::GetInstance().GetDirectXRenderer()->GetD3D11DeviceContext()->PSSetShader(pixelShader, NULL, 0);
    Editor::GetInstance().GetDirectXRenderer()->GetD3D11DeviceContext()->PSSetConstantBuffers(1, 1, &channelControlConstantBufferGpu2);
}
void TextureViewerPanel::CalculateTextureDimensions(unsigned int& width, unsigned int& height) const
{
    width = textureWidth;
    height = textureHeight;
    if (zoomMode == ZoomMode::Fit || zoomMode == ZoomMode::Fill)
    {
        const int maxWidth = Math::Max(static_cast<int>(viewportSize.x), 0);
        const int maxHeight = Math::Max(static_cast<int>(viewportSize.y), 0);
        if (maxWidth * textureHeight < maxHeight * textureWidth)
        {
            width = maxWidth;
            height = Math::DivideAndRoundNearest(width * textureHeight, textureWidth);
        }
        else
        {
            height = maxHeight;
            width = Math::DivideAndRoundNearest(height * textureWidth, textureHeight);
        }
        if (zoomMode == ZoomMode::Fit && (textureWidth < width || textureHeight < height))
        {
            width = textureWidth;
            height = textureHeight;
        }
    }
    else
    {
        width = static_cast<int>(textureWidth * zoom);
        height = static_cast<int>(textureHeight * zoom);
    }
}
void TextureViewerPanel::SetCustomZoomLevel(double zoomValue)
{
    double logZoom = log2(zoomValue);
    double quantizationSteps = zoomFactorLogSteps * 2.0;
    double logZoomQuantized = (1.0 / quantizationSteps) * (double)Math::RoundToInt(quantizationSteps * logZoom);
    zoomValue = pow(2.0, logZoomQuantized);
    zoomValue = Math::Clamp(zoomValue, minZoom, maxZoom);
    zoom = zoomValue;
    zoomMode = ZoomMode::Custom;
}
void TextureViewerPanel::ZoomIn()
{
    const double currentZoom = CalculateDisplayedZoomLevel();
    SetCustomZoomLevel(currentZoom * zoomFactor);
}
void TextureViewerPanel::ZoomOut()
{
    const double currentZoom = CalculateDisplayedZoomLevel();
    SetCustomZoomLevel(currentZoom / zoomFactor);
}
void TextureViewerPanel::UpdateZoomSliderValue()
{
    double zoomValue = CalculateDisplayedZoomLevel();
    double octaves = log2(maxZoom / minZoom);
    double sliderValue = log2(zoomValue / minZoom) / octaves;
    *this->sliderValue = (float)sliderValue;
}