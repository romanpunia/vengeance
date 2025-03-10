#include "internal/layouts_shape.hlsl"
#include "internal/channels_immediate.hlsl"
#include "internal/buffers_object.hlsl"

VOutput vs_main(VInput V)
{
	VOutput Result = (VOutput)0;
	Result.Position = mul(float4(V.Position, 1.0), ob_Transform);
	Result.Texcoord.xy = V.Texcoord;

	return Result;
}

float4 ps_main(VOutput V) : SV_TARGET0
{
	float4 Color = GetDiffuse(V.Texcoord.xy);
	return float4(Color.xyz * ob_Texcoord.xyz, Color.w);
};