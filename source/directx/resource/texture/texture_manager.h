#pragma once

#include"texture.h"

// 前方宣言
class ResourceManager;

class TextureManager
{
public:
    TextureManager(
        ID3D12Device* device,
        ID3D12CommandQueue* commandQueue,
        ResourceManager* resourceManager
    );

    bool Init();

    Texture* LoadTexture(const std::wstring& fileName);

    /// <summary>
    /// SRV用DescriptorHeapを取得（ResourceManagerから取得）
    /// </summary>
    ID3D12DescriptorHeap* GetSRVDescriptorHeap() const;

private:
    ID3D12Device* m_Device = nullptr;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList = nullptr;
    ID3D12CommandQueue* m_CommandQueue = nullptr;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator = nullptr;
    ResourceManager* m_ResourceManager = nullptr; // ResourceManagerへの参照

    std::unordered_map<std::wstring, std::unique_ptr<Texture>> m_Textures;
};