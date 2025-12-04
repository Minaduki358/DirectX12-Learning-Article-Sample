#pragma once

#include"rendertexture.h"
#include"depthstenciltexture.h"

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
    /// DepthStencilTextureを作成
    /// </summary>
    DepthStencilTexture* CreateDepthStencilTexture(const std::string name, UINT width, UINT height, DXGI_FORMAT format, bool createSRV = true);

    /// <summary>
    /// 名前でRenderTextureを取得
    /// </summary>
    RenderTexture* GetRenderTexture(const std::string& name);

    /// <summary>
    /// 名前でDepthStencilTextureを取得
    /// </summary>
    DepthStencilTexture* GetDepthStencilTexture(const std::string& name);

private:
    ID3D12Device* m_Device = nullptr;
    ResourceManager* m_ResourceManager = nullptr;

    std::unordered_map<std::string, std::unique_ptr<RenderTexture>> m_RenderTextures;
    std::unordered_map<std::string, std::unique_ptr<DepthStencilTexture>> m_DepthStencilTextures;
};