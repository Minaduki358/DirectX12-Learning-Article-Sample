#pragma once

#include"texture.h"

class TextureManager
{
public:
    TextureManager(
        ID3D12Device* device,
        ID3D12GraphicsCommandList* commandList,
        ID3D12CommandQueue* commandQueue,
        ID3D12CommandAllocator* commandAllocator
    );

    bool Init();

    Texture* LoadTexture(const std::wstring& fileName);

    /// <summary>
    /// SRV用DescriptorHeapを取得
    /// </summary>
    ID3D12DescriptorHeap* GetSRVDescriptorHeap() const { return m_SRVDescriptorHeap.Get(); }

private:
    ID3D12Device* m_Device = nullptr;
    ID3D12GraphicsCommandList* m_CommandList = nullptr;
    ID3D12CommandQueue* m_CommandQueue = nullptr;
    ID3D12CommandAllocator* m_CommandAllocator = nullptr;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_SRVDescriptorHeap = nullptr; // SRV用
    std::unordered_map<std::wstring, std::unique_ptr<Texture>> m_Textures;
    UINT m_CurrentDescriptorIndex = 0;
    UINT m_DescriptorSize = 0; // Descriptorのサイズ
};