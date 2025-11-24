#include"deferred_header.hlsli"

float4 main(Varyings IN) : SV_TARGET
{
    return float4(Texture.Sample(Sampler, IN.uv));
}