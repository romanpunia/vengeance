#include "internal/layouts_shape.hlsl"
#include "internal/channels_effect.hlsl"
#include "internal/utils_random.hlsl"

cbuffer RenderBuffer : register(b3)
{
	float3 Padding;
	float Power;
	float2 Texel;
	float Samples;
	float Blur;
}

Texture2D ImageBuffer : register(t5);

VOutput vs_main(VInput V)
{
	VOutput Result = (VOutput)0;
	Result.Position = float4(V.Position, 1.0);
	Result.Texcoord = Result.Position;

	return Result;
}

float4 ps_main(VOutput V) : SV_TARGET0
{
    float2 UV = GetTexcoord(V.Texcoord);
	float3 B = float3(0, 0, 0);

	[loop] for (int i = 0; i < Samples; i++)
	{
		float2 T = UV + float2(Gaussian[i].x, 0) * Texel * Blur;
		B += ImageBuffer.SampleLevel(Sampler, T, 0).xyz;
	}

	return float4(B * Power / Samples, 1.0);
};