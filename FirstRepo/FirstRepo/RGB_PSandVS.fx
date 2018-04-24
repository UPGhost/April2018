
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

cbuffer ConstantBuffer : register (b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 Color;
}

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
};



//--------------------------------------------------------------------------------------
// Vertex Shader PROPERTIES?
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.Pos = mul(input.Pos, World);
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);

	return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader COLOURS
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	return Color;
}