#include "camera.h"

using namespace DirectX;

Camera::Camera()
    : m_Position(0.0f, 0.0f, -5.0f)
    , m_Target(0.0f, 0.0f, 0.0f)
    , m_Up(0.0f, 1.0f, 0.0f)
    , m_ViewMatrix(XMMatrixIdentity())
    , m_ProjectionMatrix(XMMatrixIdentity())
    , m_ModelMatrix(XMMatrixIdentity())
    , m_IsDirty(true)
{
}

Camera::~Camera()
{
}

void Camera::SetPosition(const DirectX::XMFLOAT3& position)
{
    m_Position = position;
    m_IsDirty = true;
}

void Camera::SetLookAt(const DirectX::XMFLOAT3& target)
{
    m_Target = target;
    m_IsDirty = true;
}

void Camera::SetUp(const DirectX::XMFLOAT3& up)
{
    m_Up = up;
    m_IsDirty = true;
}

void Camera::SetProjection(float fov, float aspect, float nearPlane, float farPlane)
{
    m_ProjectionMatrix = XMMatrixPerspectiveFovLH(fov, aspect, nearPlane, farPlane);
}

void Camera::SetModelMatrix(const DirectX::XMMATRIX& modelMatrix)
{
    m_ModelMatrix = modelMatrix;
}

void Camera::Update()
{
    if (m_IsDirty)
    {
        XMVECTOR pos = XMLoadFloat3(&m_Position);
        XMVECTOR target = XMLoadFloat3(&m_Target);
        XMVECTOR up = XMLoadFloat3(&m_Up);

        m_ViewMatrix = XMMatrixLookAtLH(pos, target, up);
        m_IsDirty = false;
    }
}


