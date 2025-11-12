#include"forwardrender_test_scene.h"
#include"../../directx/renderpass/forward_render_pass.h"

ForwardRenderTestScene::ForwardRenderTestScene()
{
}

ForwardRenderTestScene::~ForwardRenderTestScene()
{
}

bool ForwardRenderTestScene::Init()
{
	RegisterRenderPass<ForwardRenderPass>(m_Camera.get());

	return true;
}

void ForwardRenderTestScene::Uninit()
{
}

void ForwardRenderTestScene::Update()
{
}
