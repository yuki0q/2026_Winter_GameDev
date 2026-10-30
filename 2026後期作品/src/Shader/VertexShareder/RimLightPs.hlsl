// VS/PS共通
#include "../Common/VertexToPixelHeader.hlsli"
 
// IN
#define PS_INPUT VertexToPixelLit
 
// PS
#include "../Common/Pixel/PixelShader3DHeader.hlsli"

cbuffer cbParam : register(b4)
{
    float4 g_diff_color;
    float4 g_ambient_color;
    float3 g_light_direction;
    float g_specular_pow;
    float3 g_camera_pos;
    float dummy;
}

float4 main(PS_INPUT PSInput) : SV_TARGET0
{   
    float4 color;
    
	// テクスチャーの色を取得
    color = diffuseMapTexture.Sample(diffuseMapSampler, PSInput.uv);
    if (color.a < 0.01f)
    {
        discard;
    }
    
    float3 norm = PSInput.normal;
    
    float difDot = dot(norm, -g_light_direction);
    
    float3 toEyes = normalize(g_camera_pos - PSInput.worldPos);
    
    //float3 reflectDir = normalize(g_light_direction + (norm * 2.0f));
    float3 rimDot = dot(norm, toEyes);
    rimDot = abs(rimDot);
    rimDot = 1.0f - rimDot;
    rimDot = pow(rimDot, 2.0f);
    
    float4 rimColor = float4(0.3f, 0.3f, 1.0f, 1.0f);
    
    float3 diffuse = color.rgb * (difDot * g_diff_color.rgb);
    
    float3 rgb = diffuse + (rimDot * rimColor.rgb) + g_ambient_color.rgb;
   
    return float4(rgb, color.a);

}