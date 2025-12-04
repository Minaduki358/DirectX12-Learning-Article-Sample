#include"deferred_header.hlsli"

PSOutput main(Varyings IN)
{
    PSOutput output;
    output.color = float4(Texture.Sample(Sampler, IN.uv));
    output.normal = float4(1, 1, 1, 1);
    return output;
}