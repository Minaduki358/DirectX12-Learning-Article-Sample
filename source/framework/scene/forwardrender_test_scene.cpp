#include"forwardrender_test_scene.h"
#include"../../directx/renderpass/gbuffer_render_pass.h"
#include"../../directx/renderpass/lighting_render_pass.h"
#include"../../directx/renderpass/final_blit_render_pass.h"

ForwardRenderTestScene::ForwardRenderTestScene()
{
}

ForwardRenderTestScene::~ForwardRenderTestScene()
{
}

bool ForwardRenderTestScene::Init()
{
	float aspectRatio = static_cast<float>(SystemData::k_ScreenWidth) / static_cast<float>(SystemData::k_ScreenHeight);
	m_Camera->SetProjection(DirectX::XM_PIDIV4, aspectRatio, 0.1f, 100.0f);
	m_Camera->Update();

	RegisterRenderPass<GBufferRenderPass>(m_Camera.get());

	RegisterRenderPass<LightingRenderPass>(m_Camera.get());

	RegisterRenderPass<FinalBlitRenderPass>(m_Camera.get());

	return true;
}

void ForwardRenderTestScene::Uninit()
{
}

void ForwardRenderTestScene::Update()
{
}
