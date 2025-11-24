#pragma once

/// <summary>
/// リソースマネージャー
/// Note : デスクリプタヒープの一元管理とスタックアロケーター方式での割り当てを行う
/// </summary>
class ResourceManager
{
public:
    /// <summary>
    /// デスクリプタタイプ
    /// </summary>
    enum class DescriptorType
    {
        SRV,
        CBV,
        UAV
    };

    /// <summary>
    /// 割り当て結果
    /// </summary>
    struct AllocationResult
    {
        // 割り当てられたインデックス
        UINT index;
        // 成功したかどうか
        bool success;
    };

    ResourceManager();
    ~ResourceManager();

    /// <summary>
    /// 初期化
    /// </summary>
    bool Init(ID3D12Device* device, UINT maxDescriptors = 1000);

    /// <summary>
    /// デスクリプタを割り当て（タイプ別カウンターを使用）
    /// </summary>
    AllocationResult AllocateDescriptor(DescriptorType type, UINT count = 1);

    /// <summary>
    /// SRVデスクリプタを割り当て
    /// </summary>
    AllocationResult AllocateSRV(UINT count = 1) { return AllocateDescriptor(DescriptorType::SRV, count); }

    /// <summary>
    /// CBVデスクリプタを割り当て
    /// </summary>
    AllocationResult AllocateCBV(UINT count = 1) { return AllocateDescriptor(DescriptorType::CBV, count); }

    /// <summary>
    /// UAVデスクリプタを割り当て
    /// </summary>
    AllocationResult AllocateUAV(UINT count = 1) { return AllocateDescriptor(DescriptorType::UAV, count); }

    /// <summary>
    /// CBV_SRV_UAVデスクリプタヒープを取得
    /// </summary>
    ID3D12DescriptorHeap* GetCBVSRVUAVHeap() const { return m_CBVSRVUAVHeap.Get(); }

    /// <summary>
    /// デスクリプタサイズを取得
    /// </summary>
    UINT GetDescriptorSize() const { return m_DescriptorSize; }

    /// <summary>
    /// CPUデスクリプタハンドルを取得
    /// </summary>
    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(UINT index) const;

    /// <summary>
    /// GPUデスクリプタハンドルを取得
    /// </summary>
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(UINT index) const;

    /// <summary>
    /// 使用状況を取得
    /// </summary>
    UINT GetUsedCount(DescriptorType type) const;

    /// <summary>
    /// 総使用数を取得
    /// </summary>
    UINT GetTotalUsedCount() const { return m_CurrentIndex; }

    /// <summary>
    /// 総残り使用可能数を取得
    /// </summary>
    UINT GetTotalRemainingCount() const { return m_MaxDescriptors - m_CurrentIndex; }

    ///----------------------------------------RTV&DSV---------------------------------------------------------///

    /// <summary>
    /// RTVヒープを取得
    /// </summary>
    ID3D12DescriptorHeap* GetRTVHeap() const { return m_RTVHeap.Get(); }

    /// <summary>
    /// DSVヒープを取得
    /// </summary>
    ID3D12DescriptorHeap* GetDSVHeap() const { return m_DSVHeap.Get(); }

    /// <summary>
    /// RTVデスクリプタを割り当て
    /// </summary>
    AllocationResult AllocateRTV(UINT count = 1);

    /// <summary>
    /// DSVデスクリプタを割り当て
    /// </summary>
    AllocationResult AllocateDSV(UINT count = 1);

    /// <summary>
    /// RTVのCPUハンドルを取得
    /// </summary>
    D3D12_CPU_DESCRIPTOR_HANDLE GetRTVHandle(UINT index) const;

    /// <summary>
    /// DSVのCPUハンドルを取得
    /// </summary>
    D3D12_CPU_DESCRIPTOR_HANDLE GetDSVHandle(UINT index) const;

private:
    /// <summary>
    /// タイプ別カウンター
    /// </summary>
    struct TypeCounter
    {
        // 現在の使用数
        UINT current = 0;
        // 最大数
        UINT max = 0;
    };

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CBVSRVUAVHeap = nullptr;
    ID3D12Device* m_Device = nullptr;
    UINT m_DescriptorSize = 0;
    // スタックアロケーターの現在位置
    UINT m_CurrentIndex = 0;
    // 最大デスクリプタ数
    UINT m_MaxDescriptors = 1000;

    // タイプ別カウンター
    TypeCounter m_SRVCounter;
    TypeCounter m_CBVCounter;
    TypeCounter m_UAVCounter;

    ///----------------------------------------RTV&DSV---------------------------------------------------------///

    // ★ 追加：RTVヒープとDSVヒープ ★
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RTVHeap = nullptr;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DSVHeap = nullptr;

    UINT m_RTVDescriptorSize = 0;
    UINT m_DSVDescriptorSize = 0;
    UINT m_RTVCurrentIndex = 0;
    UINT m_DSVCurrentIndex = 0;
    UINT m_MaxRTVDescriptors = 100;
    UINT m_MaxDSVDescriptors = 100;
};

