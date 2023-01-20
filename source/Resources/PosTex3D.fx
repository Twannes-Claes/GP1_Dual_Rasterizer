//-------------------------
//	Globals
//-------------------------

float gPI = 3.14159265359f;
float gKD = 1.f;
float gLightIntensity = 7.0f;
float gShininess = 25.0f;

float3 gLightDirection = float3(0.577f, -0.577f, 0.577f);

float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldMatrix : WorldMatrix;
float4x4 gInverseViewMatrix : InverseViewMatrix;

Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinesMap;

float3 gAmbientColor = float3(0.025f, 0.025f, 0.025f);

//SamplerState
SamplerState gSampler : Sampler
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap; //or Mirror, Clamp, Border
	AddressV = Wrap; //or Mirror, Clamp, Border
};

RasterizerState gRasterizerState
{
	CullMode = none;
	FrontCounterClockWise = false; //default
};

BlendState gBlendState
{
	BlendEnable[0] = false;
	RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = 1;
	DepthFunc = less;
	StencilEnable = false;

	//others are redundant because
	//StencilEnable is false
	//(for demo purposes only)
	StencilReadMask = 0x0F;
	StencilWriteMask = 0x0F;

	FrontFaceStencilFunc = always;
	BackFaceStencilFunc = always;

	FrontFaceStencilDepthFail = keep;
	BackFaceStencilDepthFail = keep;

	FrontFaceStencilPass = keep;
	BackFaceStencilPass = keep;

	FrontFaceStencilFail = keep;
	BackFaceStencilFail = keep;
};

//-------------------------
//	Input/Output Structs
//-------------------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 UV : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : COLOR;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 UV : TEXCOORD;
};

//-------------------------
//	PixelShading function
//-------------------------

float3 PixelShading(VS_OUTPUT input)
{
	//calculate the normal
	float3 sampledNormal = input.Normal;

	//calculate the sampledNormal value
	const float3 binormal = cross(sampledNormal, input.Tangent);
	const float3x3 tangentSpaceAxis = { input.Tangent, normalize(binormal), sampledNormal };

	const float4 colorNormal = gNormalMap.Sample(gSampler, input.UV);
	sampledNormal = colorNormal.rgb;

	sampledNormal = (2 * sampledNormal) - float3(1.f, 1.f, 1.f);
	sampledNormal = mul(sampledNormal, tangentSpaceAxis);

	//calculate the observedArea
	const float observedArea = saturate(dot(sampledNormal, -gLightDirection));

	//calculate the diffuse color
	float4 diffuseColor = gDiffuseMap.Sample(gSampler, input.UV);
	diffuseColor = (diffuseColor * gKD / gPI) * gLightIntensity;

	//calculate the specular color
	//calculate the viewDirection
	const float3 viewDirection = normalize(input.WorldPosition.xyz - gInverseViewMatrix[3].xyz);

	//calculations for the specular color
	const float3 reflectVector = reflect(gLightDirection, sampledNormal);
	const float reflectAngle = saturate(dot(reflectVector, -viewDirection));

	const float4 glossColor = gGlossinessMap.Sample(gSampler, input.UV);
	const float exponent = glossColor.r * gShininess;

	const float phongValue = pow(reflectAngle, exponent);

	//fill in the specular color
	const float4 specularColor = gSpecularMap.Sample(gSampler, input.UV) * phongValue;

	const float3 finalColor = (diffuseColor.rgb * observedArea) + specularColor.rgb;

	//return the final color with the ambient color
	return finalColor + gAmbientColor;
}

//-------------------------
//	Vertex Shader
//-------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.WorldPosition = mul(float4(input.Position, 1.0f), gWorldMatrix);
	output.Tangent = mul(normalize(input.Tangent), (float3x3)gWorldMatrix);
	output.Normal = mul(normalize(input.Normal), (float3x3)gWorldMatrix);
	output.UV = input.UV;
	return output;
}

//-------------------------
//	Pixel Shader
//-------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	//return gDiffuseMap.Sample(gSampler, input.UV);

	float4 pixelShade = float4(PixelShading(input), 1.f);

	return pixelShade;
}

//-------------------------
//	Technique
//-------------------------
technique11 DefaultTechnique
{
	pass P0
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
