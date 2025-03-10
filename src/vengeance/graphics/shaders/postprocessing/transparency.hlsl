#include "internal/layouts_shape.hlsl"
#include "internal/channels_effect.hlsl"
#include "internal/utils_position.hlsl"
#pragma warning(disable: 4000)

cbuffer TransparencyBuffer : register(b3)
{
	float3 Padding;
	float Mips;
}

Texture2D LDiffuseBuffer : register(t5);
Texture2D LNormalBuffer : register(t6);
Texture2D LDepthBuffer : register(t7);
Texture2D LSurfaceBuffer : register(t8);

VOutput vs_main(VInput V)
{
	VOutput Result = (VOutput)0;
	Result.Position = float4(V.Position, 1.0);
	Result.Texcoord = Result.Position;

	return Result;
}

float4 ps_main(VOutput V) : SV_TARGET0
{
	float2 Texcoord = GetTexcoord(V.Texcoord);
	float D1 = LDepthBuffer.SampleLevel(Sampler, Texcoord, 0).x;
	float D2 = GetDepth(Texcoord);
	[branch] if (D2 < D1 || D1 >= 1.0)
		return float4(GetDiffuse(Texcoord, 0.0).xyz, 1.0);

	float4 Normal = LNormalBuffer.SampleLevel(Sampler, Texcoord, 0);
	Material Mat = Materials[Normal.w];
	float4 Diffuse = LDiffuseBuffer.SampleLevel(Sampler, Texcoord, 0);
	float A = min(1.0, (1.0 - Diffuse.w) + Mat.Transparency);
	float R = max(0.0, Mat.Roughness.x + LSurfaceBuffer.SampleLevel(Sampler, Texcoord, 0).x * Mat.Roughness.y - 0.25) / 0.75;
	float2 F = Texcoord + (Mat.Refraction * length(Normal.xyz) - Mat.Refraction * 1.2);
	return float4(Diffuse.xyz * GetDiffuse(F, R * Mips).xyz * A, 1.0);
};