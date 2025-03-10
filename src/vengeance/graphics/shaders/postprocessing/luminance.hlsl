#include "internal/layouts_shape.hlsl"
#include "internal/channels_effect.hlsl"
#include "internal/utils_random.hlsl"

cbuffer RenderBuffer : register(b3)
{
	float2 Texel;
	float Mips;
	float Time;
}

Texture2D LutBuffer : register(t6);

float GetAvg(float2 Texcoord)
{
	float3 Color = GetDiffuseSample(Texcoord).xyz;
	return (Color.x + Color.y + Color.z) / 3.0;
}

VOutput vs_main(VInput V)
{
	VOutput Result = (VOutput)0;
	Result.Position = float4(V.Position, 1.0);
	Result.Texcoord.xy = V.Texcoord;

	return Result;
}

float ps_main(VOutput V) : SV_TARGET0
{
	float Result = 0.0;
	[unroll] for (float i = 0; i < 16.0; i++)
		Result += GetAvg(V.Texcoord.xy + Gaussian[i] * Texel);
	
	float Subresult = LutBuffer.SampleLevel(Sampler, V.Texcoord.xy, 0).r;
	return Subresult + (Result / 4.0 - Subresult) * (1.0 - exp(-Time));
};