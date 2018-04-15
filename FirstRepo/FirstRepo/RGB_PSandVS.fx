
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

cbuffer ConstantBuffer : register (b0)
{
	matrix World;
	matrix View;
	matrix Projection;
}

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
    float4 Pos : POSITION;
    float4 Color : COLOR;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 Color : COLOR;
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
	output.Color = input.Color;

	return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader COLOURS
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	//return float4(1, 0, 0, 1); //R, G, B, A			

	//return float4(1, 0, 0, 1); For Red
	//return float4(input.Color.r, input.Color.g, input.Color.b, 1);
	
	return input.Color;
}

