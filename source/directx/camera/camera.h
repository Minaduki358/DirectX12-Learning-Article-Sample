#pragma once

#include"camera_data.h"

/// <summary>
/// カメラクラス
/// </summary>
class Camera
{
public:
    Camera();
    ~Camera();

    /// <summary>
    /// 位置を設定
    /// </summary>
    void SetPosition(const DirectX::XMFLOAT3& position);

    /// <summary>
    /// 注視点を設定
    /// </summary>
    void SetLookAt(const DirectX::XMFLOAT3& target);

    /// <summary>
    /// 上方向を設定
    /// </summary>
    void SetUp(const DirectX::XMFLOAT3& up);

    /// <summary>
    /// プロジェクション行列を設定
    /// </summary>
    void SetProjection(float fov, float aspect, float nearPlane, float farPlane);

    /// <summary>
    /// CameraDataを取得
    /// </summary>
    const CameraData& GetCameraData() const { return m_CameraData; }

    /// <summary>
    /// ビュー行列を取得
    /// </summary>
    DirectX::XMMATRIX GetViewMatrix() const { return m_CameraData.ViewMatrix; }

    /// <summary>
    /// プロジェクション行列を取得
    /// </summary>
    DirectX::XMMATRIX GetProjectionMatrix() const { return m_CameraData.ProjectionMatrix; }

    /// <summary>
    /// カメラ行列を更新（ビュー行列を再計算）
    /// </summary>
    void Update();

private:
    DirectX::XMFLOAT3 m_Position;
    DirectX::XMFLOAT3 m_Target;
    DirectX::XMFLOAT3 m_Up;

    CameraData m_CameraData;
    bool m_IsDirty;
};
