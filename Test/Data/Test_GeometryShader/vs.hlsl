struct VSInputOutput
{
    float3 pos : POSITION;
    float size : SIZE;
};

VSInputOutput main(VSInputOutput input)
{
    return input;
}
