#pragma once

#include"scene.h"

class DeferredRenderTestScene : public Scene
{
public:
	DeferredRenderTestScene();
	~DeferredRenderTestScene() override;

	bool Init() override;
	void Uninit() override;
	void Update() override;
};