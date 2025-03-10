#include "internal/layouts_shape.hlsl"
#include "internal/channels_effect.hlsl"
#include "internal/utils_position.hlsl"

cbuffer RenderBuffer : register(b3)
{
	float2 Padding;
	float Power;
	float Threshold;
}

Texture2D PrevDiffuseBuffer : register(t6);

VOutput vs_main(VInput V)
{
	VOutput Result = (VOutput)0;
	Result.Position = float4(V.Position, 1.0);
	Result.Texcoord.xy = V.Texcoord;
	return Result;
}

float4 ps_main(VOutput V) : SV_TARGET0
{
	float3 PrevColor = PrevDiffuseBuffer.SampleLevel(Sampler, V.Texcoord.xy, 0).xyz;
	float3 NextColor = GetDiffuse(V.Texcoord.xy, 0).xyz;
	float3 DeltaColor = PrevColor - NextColor;
    float DeltaX = dot(float3(1.0, 0.0, 0.0), DeltaColor);
    float DeltaY = dot(float3(0.0, 1.0, 0.0), DeltaColor);
	DeltaX = clamp(Power * abs(DeltaX) - Threshold, 0, 1) * sign(DeltaX);
	DeltaY = clamp(Power * abs(DeltaY) - Threshold, 0, 1) * sign(DeltaY);
	return float4(DeltaX * 0.5 + 0.5, DeltaY * 0.5 + 0.5, 0.0, 1.0f);
};