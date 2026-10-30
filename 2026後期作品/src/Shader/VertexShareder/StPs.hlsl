// VS/PS共通
#include "../Common/VertexToPixelHeader.hlsli"
 
// IN
#define PS_INPUT VertexToPixelLit
 
// PS
#include "../Common/Pixel/PixelShader3DHeader.hlsli"

cbuffer cbParam : register(b4)
{
    float3 g_light_direction;
    float dummy;
}

float4 main(PS_INPUT PSInput) : SV_TARGET0
{
    //return float4(
    //PSInput.normal.x, PSInput.normal.x, PSInput.normal.x, 1.0f);
   // return float4(g_light_direction, 1.0f);
    
    float4 color;
 
    //color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
	// テクスチャーの色を取得
    color = diffuseMapTexture.Sample(diffuseMapSampler, PSInput.uv);
    if (color.a < 0.01f)
    {
        discard;
    }
 
	// 関数の戻り値がラスタライザに渡される
   // return color;
    //float3 lightDir = g_common.light[0].direction;
    
    float lightDot = dot(PSInput.normal, -g_light_direction);
    float3 rgb = color.rgb * lightDot;
    rgb += float3(0.2f, 0.2f, 0.2f);
    return float4(rgb, color.a);
    
    //return float4(lightDir.x, lightDir.y, lightDir.z, 1.0f);
    //return float4(g_light_dir.x, g_light_dir.y, g_light_dir.z, 1.0f);

}