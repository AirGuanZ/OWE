cbuffer Trans
{
    float4x4 VP;
};

struct GSInput
{
    float3 pos : POSITION;
    float size : SIZE;
};

struct GSOutput
{
    float4 pos : SV_POSITION;
};

[maxvertexcount(6)]
void main(point GSInput input[1], inout TriangleStream<GSOutput> output)
{
    GSOutput pnt = (GSOutput)0;
    float3 offset = input[0].pos;
    float sz = 0.5f * input[0].size;

    pnt.pos = mul(float4(offset + float3(-sz, -sz, 0.0f), 1.0f), VP);
    output.Append(pnt);
    pnt.pos = mul(float4(offset + float3(-sz, +sz, 0.0f), 1.0f), VP);
    output.Append(pnt);
    pnt.pos = mul(float4(offset + float3(+sz, +sz, 0.0f), 1.0f), VP);
    output.Append(pnt);

    output.RestartStrip();

    pnt.pos = mul(float4(offset + float3(-sz, -sz, 0.0f), 1.0f), VP);
    output.Append(pnt);
    pnt.pos = mul(float4(offset + float3(+sz, +sz, 0.0f), 1.0f), VP);
    output.Append(pnt);
    pnt.pos = mul(float4(offset + float3(+sz, -sz, 0.0f), 1.0f), VP);
    output.Append(pnt);
}
