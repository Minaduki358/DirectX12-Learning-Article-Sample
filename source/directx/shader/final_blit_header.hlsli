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

Texture2D<float4> Texture : register(t0); // 0番スロットに設定されたテクスチャ
SamplerState Sampler : register(s0); // 0番スロットに設定されたサンプラー