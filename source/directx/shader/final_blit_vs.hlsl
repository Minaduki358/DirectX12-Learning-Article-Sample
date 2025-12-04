#include"final_blit_header.hlsli"

Varyings main(uint vertexID : SV_VertexID)
{
    Varyings output;
    
    // 3頂点で画面全体を覆う大きな三角形
    // note: https://www.gamedev.net/forums/topic/609917-full-screen-quad-without-vertex-buffer/page__p__4857328#entry4857328
    output.uv = float2((vertexID << 1) & 2, vertexID & 2);
    output.position = float4(output.uv * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    
    return output;
}