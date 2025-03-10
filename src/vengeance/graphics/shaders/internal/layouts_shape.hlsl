struct VInput
{
	float3 Position : POSITION;
	float2 Texcoord : TEXCOORD0;
};

struct VOutput
{
	float4 Position : SV_POSITION;
	float4 Texcoord : TEXCOORD0;
};