// VS/PS‹¤’Ê
#include "../Common/VertexToPixelHeader.hlsli"
 
// IN
#define PS_INPUT VertexToPixelLit
 
// PS
#include "../Common/Pixel/PixelShader3DHeader.hlsli"

cbuffer cbParam : register(b4)
{
    float4 g_diffuse;
}

float4 main(PS_INPUT PSInput) : SV_TARGET0
{
    return g_diffuse; 
}