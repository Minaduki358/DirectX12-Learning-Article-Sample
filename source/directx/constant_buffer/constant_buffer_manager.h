#pragma once

#include "constant_buffer.h"

// 前方宣言
class ResourceManager;

/// <summary>
/// コンスタントバッファマネージャー
/// </summary>
class ConstantBufferManager
{
public:
    ConstantBufferManager(
        ID3D12Device* device,
        ResourceManager* resourceManager
    );

    /// <summary>
    /// 初期化
    /// </summary>
    /// <returns>成功したかどうか</returns>
    bool Init();

    /// <summary>
    /// コンスタントバッファを作成
    /// </summary>
    /// <param name="size">コンスタントバッファのサイズ（バイト）</param>
    /// <returns>作成されたコンスタントバッファ（所有権はマネージャーが保持）</returns>
    ConstantBuffer* CreateConstantBuffer(UINT size);

private:
    ID3D12Device* m_Device = nullptr;
    ResourceManager* m_ResourceManager = nullptr;
    std::vector<std::unique_ptr<ConstantBuffer>> m_ConstantBuffers;
};


