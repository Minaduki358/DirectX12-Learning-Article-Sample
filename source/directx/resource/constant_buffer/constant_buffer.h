#pragma once

#include <d3d12.h>
#include <wrl/client.h>

// 前方宣言
class ResourceManager;

/// <summary>
/// コンスタントバッファクラス
/// </summary>
class ConstantBuffer
{
public:
    ConstantBuffer();
    ~ConstantBuffer();

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="device">D3D12デバイス</param>
    /// <param name="resourceManager">リソースマネージャー</param>
    /// <param name="size">コンスタントバッファのサイズ（バイト、256バイトアライメント）</param>
    /// <returns>成功したかどうか</returns>
    bool Init(ID3D12Device* device, ResourceManager* resourceManager, UINT size);

    /// <summary>
    /// データを更新（CPUからGPUへ転送）
    /// </summary>
    /// <param name="data">更新するデータ</param>
    /// <param name="size">データサイズ（バイト）</param>
    void UpdateData(const void* data, UINT size);

    /// <summary>
    /// GPUデスクリプタハンドルを取得
    /// </summary>
    /// <returns>GPUデスクリプタハンドル</returns>
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle() const { return m_GPUDescriptorHandle; }

    /// <summary>
    /// コンスタントバッファのサイズを取得
    /// </summary>
    /// <returns>サイズ（バイト）</returns>
    UINT GetSize() const { return m_Size; }

private:
    ID3D12Device* m_Device = nullptr;
    ResourceManager* m_ResourceManager = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_ConstantBuffer = nullptr;
    void* m_MappedData = nullptr;
    UINT m_Size = 0;
    D3D12_GPU_DESCRIPTOR_HANDLE m_GPUDescriptorHandle = {};
};

