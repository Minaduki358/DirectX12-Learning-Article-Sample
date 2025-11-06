#pragma once

#include <DirectXMath.h>

/// <summary>
/// シェーダーに送るカメラデータ構造体
/// </summary>
struct CameraData
{
    DirectX::XMMATRIX ModelMatrix;
    DirectX::XMMATRIX ViewMatrix;
    DirectX::XMMATRIX ProjectionMatrix;
};


