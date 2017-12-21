cbuffer Trans
{
    float4x4 WVP;
};

Texture2D<float4> heightMap;
SamplerState sam;

struct VSInput
{
    float2 pos : POSITION;
    float2 uv : TEXCOORD;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.pos = mul(float4(input.pos.x, 10.0f * heightMap.SampleLevel(sam, input.uv, 0).r, input.pos.y, 1.0f), WVP);
    output.uv = input.uv;
    return output;
}
