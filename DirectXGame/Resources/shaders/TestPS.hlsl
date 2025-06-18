struct PixelShaderOutPut
{
    float4 color : SV_TARGET0;
};

PixelShaderOutPut main()
{
    PixelShaderOutPut output;
    output.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    return output;
}