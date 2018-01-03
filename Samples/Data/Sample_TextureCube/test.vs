cbuffer Trans
{
    float4x4 WVP;
};

struct VSInput
{
    float3 pos : POSITION;
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
    output.pos = mul(float4(input.pos, 1.0f), WVP);
    output.uv = input.uv;
    return output;
}
