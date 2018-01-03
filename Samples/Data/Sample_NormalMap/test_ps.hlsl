cbuffer DirectionalLight
{
    float3 lightPos;
    float3 lightColor;
};

struct PSInput
{
    float4 pos        : SV_POSITION;
    float2 uv         : TEXCOORD;
    float3 worldAxisU : AXIS_U;
    float3 worldAxisV : AXIS_V;
    float3 worldNor   : NORMAL;
    float3 worldPos   : WORLD_POSITION;
};

Texture2D<float3> normalMap;
Texture2D<float3> tex;

SamplerState sam;

float4 main(PSInput input) : SV_TARGET
{
    float3 norV = 2.0f * normalMap.Sample(sam, input.uv) - 1.0f;
    float3 finalNor = normalize(norV.x * input.worldAxisU +
                                norV.y * input.worldAxisV +
                                norV.z * input.worldNor);
    float lightFactor = max(0.0f, dot(finalNor, normalize(lightPos - input.worldPos)));
    return float4(lightFactor * lightColor * tex.Sample(sam, input.uv), 1.0f);
}
