#pragma once

#include <d3d12.h>
#include <type_traits>

/// <summary>
/// DirectX12用アライメントヘルパー関数
/// ConstantBuffer、テクスチャ、バッファなどのアライメント計算に使用
/// DirectX12の標準定数を使用：
/// - D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT (256)
/// - D3D12_TEXTURE_DATA_PITCH_ALIGNMENT (256)
/// - D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT (65536)
/// </summary>
namespace AlignmentHelper
{
    /// <summary>
    /// 256バイトアライメント（ConstantBuffer用）
    /// D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENTを使用
    /// </summary>
    constexpr UINT Align256(UINT size)
    {
        constexpr UINT alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
        return (size + (alignment - 1)) & ~(alignment - 1);
    }

    /// <summary>
    /// 任意のアライメント
    /// </summary>
    constexpr UINT AlignTo(UINT size, UINT alignment)
    {
        return (size + (alignment - 1)) & ~(alignment - 1);
    }

    /// <summary>
    /// 16バイトアライメント（XMMATRIX用）
    /// </summary>
    constexpr UINT Align16(UINT size)
    {
        return (size + 15) & ~15;
    }

    /// <summary>
    /// テクスチャ行ピッチアライメント
    /// D3D12_TEXTURE_DATA_PITCH_ALIGNMENTを使用
    /// </summary>
    constexpr UINT AlignTexturePitch(UINT size)
    {
        constexpr UINT alignment = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
        return (size + (alignment - 1)) & ~(alignment - 1);
    }

    /// <summary>
    /// 64KBアライメント（大きなバッファ用）
    /// D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENTを使用
    /// </summary>
    constexpr UINT Align64KB(UINT size)
    {
        constexpr UINT alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
        return (size + (alignment - 1)) & ~(alignment - 1);
    }

    /// <summary>
    /// テンプレート版：型のサイズを256バイトにアライメント
    /// </summary>
    template<typename T>
    constexpr UINT AlignedSize256()
    {
        return Align256(sizeof(T));
    }

    /// <summary>
    /// テンプレート版：型のサイズを任意にアライメント
    /// </summary>
    template<typename T>
    constexpr UINT AlignedSizeTo(UINT alignment)
    {
        return AlignTo(sizeof(T), alignment);
    }

    /// <summary>
    /// 構造体が16バイトアライメントされているか検証
    /// ConstantBufferに使用する構造体に推奨
    /// </summary>
    template<typename T>
    constexpr bool Is16ByteAligned()
    {
        return alignof(T) >= 16;
    }

    /// <summary>
    /// 構造体が特定のアライメント要件を満たしているか検証
    /// </summary>
    template<typename T>
    constexpr bool IsAlignedTo(size_t alignment)
    {
        return alignof(T) >= alignment;
    }

    /// <summary>
    /// ConstantBuffer用の検証（256バイトアライメント＋16バイト境界）
    /// </summary>
    template<typename T>
    constexpr bool ValidateConstantBufferAlignment()
    {
        return Is16ByteAligned<T>();
    }

    /// <summary>
    /// サイズが256の倍数かチェック
    /// </summary>
    constexpr bool Is256ByteAligned(UINT size)
    {
        return (size & 255) == 0;
    }

    /// <summary>
    /// サイズが特定のアライメントの倍数かチェック
    /// </summary>
    constexpr bool IsAlignedSize(UINT size, UINT alignment)
    {
        return (size % alignment) == 0;
    }

    /// <summary>
    /// D3D12テクスチャの行ピッチを計算
    /// D3D12_TEXTURE_DATA_PITCH_ALIGNMENT (256バイト) にアライメント
    /// </summary>
    constexpr UINT CalculateTexturePitch(UINT width, UINT bytesPerPixel)
    {
        return AlignTexturePitch(width * bytesPerPixel);
    }
}
