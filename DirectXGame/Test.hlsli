struct VertexShaderOutput
{
    float32_4 position : SV_POSITION;
    float32_4 texcoord : TEXCOORD0;
};

struct VertexShaderInput
{
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
};

struct PixelShaderOutPut
{
    float4 color : SV_TARGET0;
};