#include"basic_header.hlsli"

float4 main(Output input) : SV_TARGET
{
    return float4(tex.Sample(smp, input.uv));
}