
// ワールド行列用のコンスタントバッファ（register b0）
cbuffer TransformBuffer : register(b0)
{
    row_major float4x4 WorldMatrix;
};

// カメラデータ構造体
struct CameraData
{
    row_major matrix ViewMatrix;
    row_major matrix ProjectionMatrix;
};

// カメラデータ用のコンスタントバッファ（register b1）
cbuffer CameraDataBuffer : register(b1)
{
    CameraData cameraData;
}

struct Attributes
{
    float4 positionOS : POSITION;
    float2 uv : TEXCOORD;
};

struct Varyings
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

struct PSOutput
{
    float4 color : SV_TARGET0;
    float4 normal : SV_TARGET1;
};

Texture2D<float4> Texture : register(t0); // 0番スロットに設定されたテクスチャ
SamplerState Sampler : register(s0); // 0番スロットに設定されたサンプラー