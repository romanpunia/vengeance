struct VInput
{
	float3 Position : POSITION;
	float2 Texcoord : TEXCOORD0;
	float3 Normal : NORMAL0;
	float3 Tangent : TANGENT0;
	float3 Bitangent : BINORMAL0;
	float4 Index : JOINTBIAS0;
	float4 Bias : JOINTBIAS1;
};

struct VOutput
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
	float3 Normal : NORMAL0;
	float3 Tangent : TANGENT0;
	float3 Bitangent : BINORMAL0;
	float4 UV : TEXCOORD1;
	float3 Direction : TEXCOORD2;
};

struct VOutputCube
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
	float4 UV : TEXCOORD1;
	uint RenderTarget : SV_RenderTargetArrayIndex;
};

struct VOutputLinear
{
	float4 Position : SV_POSITION;
	float2 Texcoord : TEXCOORD0;
	float4 UV : TEXCOORD1;
};