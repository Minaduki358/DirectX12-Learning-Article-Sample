#pragma once

#include"rendertexture.h"

class ResourceManager;

class RenderTextureManager
{
public:
    RenderTextureManager(ID3D12Device* device, ResourceManager* resourceManager);
    ~RenderTextureManager();

    bool Init();

    /// <summary>
    /// RenderTextureを作成
    /// </summary>
    RenderTexture* CreateRenderTexture(const std::string& name, UINT width, UINT height, DXGI_FORMAT format, bool createSRV = true);

    /// <summary>
    /// 名前でRenderTextureを取得
    /// </summary>
    RenderTexture* GetRenderTexture(const std::string& name);

    /// <summary>
    /// RenderTextureを削除
    /// </summary>
    void RemoveRenderTexture(const std::string& name);

private:
    ID3D12Device* m_Device = nullptr;
    ResourceManager* m_ResourceManager = nullptr;

    std::unordered_map<std::string, std::unique_ptr<RenderTexture>> m_RenderTextures;
};