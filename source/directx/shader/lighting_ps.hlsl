#include"lighting_header.hlsli"

float4 main(Varyings IN) : SV_TARGET
{
    float4 albedo = AlbedoTexture.Sample(Sampler, IN.uv);
    float4 normal = NormalTexture.Sample(Sampler, IN.uv);
    return float4(albedo);
}