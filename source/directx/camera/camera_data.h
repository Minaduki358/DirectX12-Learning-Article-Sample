#pragma once

#include <DirectXMath.h>

/// <summary>
/// シェーダーに送るカメラデータ構造体
/// Note: DirectX12のコンスタントバッファは256バイトアライメントが必要
/// XMMATRIXは16バイトアライメント、構造体全体も16バイトアライメントに揃える
/// </summary>
struct alignas(16) CameraData
{
    DirectX::XMMATRIX ModelMatrix;      // 64バイト (4x4 float)
    DirectX::XMMATRIX ViewMatrix;       // 64バイト (4x4 float)
    DirectX::XMMATRIX ProjectionMatrix; // 64バイト (4x4 float)
    // 合計: 192バイト
    // ConstantBufferで256バイトにアライメントされる
};

// コンパイル時にサイズを検証
static_assert(sizeof(CameraData) == 192, "CameraData size must be 192 bytes (3 matrices * 64 bytes)");
static_assert(alignof(CameraData) == 16, "CameraData must be 16-byte aligned");


