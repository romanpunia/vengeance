#include "internal/layouts_shape.hlsl"
#include "internal/channels_effect.hlsl"
#include "internal/utils_random.hlsl"

cbuffer RenderBuffer : register(b3)
{
	float2 Texel;
	float Samples;
	float Motion;
}

Texture2D ImageBuffer : register(t5);
Texture2D VelocityBuffer : register(t6);

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
	float Velocity = (VelocityBuffer.SampleLevel(Sampler, UV, 0).y * 2.0f - 1.0f) * Motion * Texel.y;
	float3 Result = float3(0, 0, 0);
	[loop] for (float i = 0; i < Samples; i++)
	{
		float2 T = UV + float2(0, Gaussian[i].y * Velocity);
		Result += ImageBuffer.SampleLevel(Sampler, T, 0).xyz;
	}
	return float4(Result / Samples, 1.0);
};