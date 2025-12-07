#include"gbuffer_header.hlsli"

Varyings main(Attributes IN)
{
    Varyings output;
    
    // ワールド座標に変換
    float4 worldPos = mul(IN.positionOS, WorldMatrix);
    
    // ビュー座標に変換
    float4 viewPos = mul(worldPos, cameraData.ViewMatrix);
    
    // プロジェクション座標に変換
    float4 projPos = mul(viewPos, cameraData.ProjectionMatrix);
    
    output.position = projPos;
    output.uv = IN.uv;
    return output;
}