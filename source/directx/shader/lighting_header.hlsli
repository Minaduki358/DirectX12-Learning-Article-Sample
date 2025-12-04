
struct Varyings
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D<float4> AlbedoTexture : register(t0); // 0番スロットに設定されたテクスチャ
Texture2D<float4> NormalTexture : register(t1); // 1番スロットに設定されたテクスチャ
SamplerState Sampler : register(s0); // 0番スロットに設定されたサンプラー