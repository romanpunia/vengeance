#include "internal/layouts_shape.hlsl"
#include "internal/channels_effect.hlsl"

Texture2D ImageBuffer : register(t5);

VOutput vs_main(VInput V)
{
	VOutput Result = (VOutput)0;
	Result.Position = float4(V.Position, 1.0);
	Result.Texcoord.xy = V.Texcoord;

	return Result;
}

float4 ps_main(VOutput V) : SV_TARGET0
{	
	float3 A = GetDiffuse(V.Texcoord.xy, 0).xyz;
	float3 B = ImageBuffer.SampleLevel(Sampler, V.Texcoord.xy, 0).xyz;
	return float4(A * B, 1.0);
};