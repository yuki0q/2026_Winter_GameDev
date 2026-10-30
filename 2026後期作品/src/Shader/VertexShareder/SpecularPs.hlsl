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
    float4 g_specular_color;
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
    
    float3 reflectDir = normalize(g_light_direction + (norm * 2.0f));
    
    float refDot = abs(dot(reflectDir, toEyes));
    float refDotEx = pow(refDot, g_specular_pow);
    
    //float lightDot = dot(PSInput.normal, -g_light_direction);
    float3 diffuse = color.rgb * (difDot * g_diff_color.rgb);
    
    float3 rgb = diffuse + (refDotEx * g_specular_color.rgb) + g_ambient_color.rgb;
    
    return float4(rgb, color.a);

}