
// カメラデータ構造体
struct CameraData
{
    matrix ViewMatrix;
    matrix ProjectionMatrix;
};

// カメラデータ用のコンスタントバッファ（register b0）
ConstantBuffer<CameraData> cameraBuffer : register(b0);

struct Output
{
    float4 svpos : SV_POSITION; // システム用頂点座標
    float2 uv : TEXCOORD; // uv値
};

Texture2D<float4> tex : register(t0); // 0番スロットに設定されたテクスチャ
SamplerState smp : register(s0); // 0番スロットに設定されたサンプラー