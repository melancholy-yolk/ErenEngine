
cbuffer ConstantBuffer : register(b0)//b0~b14
{
	float4x4 World;
}

struct VertexInput
{
	float3 Position : POSITION;
	float4 Color : COLOR;
};

struct VertexOutput
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
};

VertexOutput VertexShaderMain(VertexInput In)
{
	VertexOutput Output;

	Output.Position = mul(float4(In.Position, 1.0f), World);
	Output.Color = In.Color;

	return Output;
}

float4 PixelShaderMain(VertexOutput In) : SV_Target
{
	return In.Color;
}