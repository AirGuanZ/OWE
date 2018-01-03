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
    VSOutput output;
    output.pos = float4(input.pos, 0.0f, 1.0f);
    output.uv = input.uv;
    return output;
}
