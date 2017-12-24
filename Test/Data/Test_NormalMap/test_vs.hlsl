cbuffer Trans
{
    float4x4 world;
    float4x4 WVP;
};

struct VSInput
{
    float3 pos   : POSITION;
    float2 uv    : TEXCOORD;
    float3 axisU : AXIS_U;
    float3 axisV : AXIS_V;
};

struct VSOutput
{
    float4 pos        : SV_POSITION;
    float2 uv         : TEXCOORD;
    float3 worldAxisU : AXIS_U;
    float3 worldAxisV : AXIS_V;
    float3 worldNor   : NORMAL;
    float3 worldPos   : WORLD_POSITION;
};

VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;

    float4 worldPos = mul(float4(input.pos, 1.0f), world);
    output.worldPos = worldPos.xyz;
    output.pos = mul(float4(input.pos, 1.0f), WVP);

    float4 wAU = mul(float4(input.axisU, 0.0f), world);
    float4 wAV = mul(float4(input.axisV, 0.0f), world);
    output.worldAxisU = wAU.xyz;
    output.worldAxisV = wAV.xyz;
    output.worldNor = normalize(cross(input.axisV, input.axisU));

    output.uv = input.uv;

    return output;
}
