#include"basic_header.hlsli"

Output main(float4 pos : POSITION, float2 uv : TEXCOORD)
{
    Output output;
    
    // ワールド座標に変換
    float4 worldPos = mul(pos, cameraBuffer.ModelMatrix);
    
    // ビュー座標に変換
    float4 viewPos = mul(worldPos, cameraBuffer.ViewMatrix);
    
    // プロジェクション座標に変換
    float4 projPos = mul(viewPos, cameraBuffer.ProjectionMatrix);
    
    output.svpos = projPos;
    output.uv = uv;
    return output;
}