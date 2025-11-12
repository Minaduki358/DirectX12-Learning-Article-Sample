#pragma once

#include"scene.h"

class ForwardRenderTestScene : public Scene
{
public:
	ForwardRenderTestScene();
	~ForwardRenderTestScene() override;

	bool Init() override;
	void Uninit() override;
	void Update() override;
};