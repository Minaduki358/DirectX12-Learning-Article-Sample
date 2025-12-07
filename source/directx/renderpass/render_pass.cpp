#include"render_pass.h"

RenderPass::RenderPass(const Camera* camera)
	:m_Camera(camera)
{
}

RenderPass::~RenderPass()
{
}

void RenderPass::SetupDefaultViewportAndScissor()
{
    // ビューポート設定
    m_ViewPort.Width = static_cast<float>(SystemData::k_ScreenWidth);
    m_ViewPort.Height = static_cast<float>(SystemData::k_ScreenHeight);
    m_ViewPort.TopLeftX = 0.0f;
    m_ViewPort.TopLeftY = 0.0f;
    m_ViewPort.MinDepth = 0.0f;

    // シザー矩形設定
    m_ScissorRec.left = 0;
    m_ScissorRec.top = 0;
    m_ScissorRec.right = SystemData::k_ScreenWidth;
    m_ScissorRec.bottom = SystemData::k_ScreenHeight;
}
