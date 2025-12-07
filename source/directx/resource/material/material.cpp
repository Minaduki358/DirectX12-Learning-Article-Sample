#include"material.h"
#include"../../renderer.h"

Material::Material()
{
}

Material::~Material()
{
}

bool Material::SetPipeline()
{
	Renderer& renderer = Renderer::GetInstance();
	return renderer.SetPipeline(m_Pipelinename);
}
