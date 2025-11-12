#pragma once

#include"../resource/resource_manager.h"
#include"../resource/constant_buffer/constant_buffer_manager.h"
#include"../resource/texture/texture_manager.h"

struct RenderContext
{
    // リソースマネージャー
    ResourceManager* resourceManager;
    ConstantBufferManager* constantBufferManager;
    TextureManager* textureManager;
};