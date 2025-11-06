#include "constant_buffer_manager.h"
#include "../resource_manager.h"

ConstantBufferManager::ConstantBufferManager(
    ID3D12Device* device,
    ResourceManager* resourceManager
)
    : m_Device(device)
    , m_ResourceManager(resourceManager)
{
}

bool ConstantBufferManager::Init()
{
    if (!m_Device || !m_ResourceManager)
    {
        return false;
    }

    return true;
}

ConstantBuffer* ConstantBufferManager::CreateConstantBuffer(UINT size)
{
    if (!m_Device || !m_ResourceManager)
    {
        return nullptr;
    }

    // コンスタントバッファインスタンスを作成
    auto constantBuffer = std::make_unique<ConstantBuffer>();
    if (!constantBuffer->Init(m_Device, m_ResourceManager, size))
    {
        return nullptr;
    }

    // マップに追加（所有権を移動）
    ConstantBuffer* constantBufferPtr = constantBuffer.get();
    m_ConstantBuffers.push_back(std::move(constantBuffer));

    return constantBufferPtr; // 非所有のポインタを返す
}

