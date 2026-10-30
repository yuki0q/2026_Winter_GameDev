// DxLib 2D用 ピクセルシェーダー
struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 dif : COLOR0;
    float4 spc : COLOR1;
    float2 uv : TEXCOORD0;
    float2 suv : TEXCOORD1;
};

SamplerState smp : register(s0);
Texture2D tex : register(t0);

cbuffer ShakeParam : register(b0)
{
    float2 offset; // UV空間でのずらし量
    float zoom; // 端の見切れ対策の拡大率(1.0以上)
    float pad;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    // 画面中心を基準に少し拡大(端のはみ出し防止)
    float2 uv = (input.uv - 0.5f) / zoom + 0.5f;
    uv += offset;
    return tex.Sample(smp, uv);
}