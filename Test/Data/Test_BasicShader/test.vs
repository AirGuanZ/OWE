cbuffer Trans
{
    float2 size;
};

float4 main(float2 iPosL : POSITION) : SV_POSITION
{
    return float4(size * iPosL, 0.2f, 1.0f);
}
