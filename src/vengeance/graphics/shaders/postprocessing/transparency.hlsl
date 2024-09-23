#include "internal/layouts_shape.hlsl"
#include "internal/channels_effect.hlsl"
#include "internal/utils_position.hlsl"
#pragma warning(disable: 4000)

cbuffer RenderConstant : register(b3)
{
	float3 Padding;
	float Mips;
}

Texture2D LDiffuseBuffer : register(t5);
Texture2D LNormalBuffer : register(t6);
Texture2D LDepthBuffer : register(t7);
Texture2D LSurfaceBuffer : register(t8);

float3 GetOpaque(float2 TexCoord, float D2, float L)
{
	[branch] if (D2 >= 1.0)
		return GetDiffuse(TexCoord, L).xyz;

	float3 Position = GetPosition(TexCoord, D2);
	float3 Eye = normalize(Position - vb_Position);
	float4 Normal = NormalBuffer.Sample(Sampler, TexCoord);

	return GetDiffuse(TexCoord, L).xyz;
}
float3 GetCoverage(float2 TexCoord, float L)
{
	return GetOpaque(TexCoord, GetDepth(TexCoord), L);
}

VOutput vs_main(VInput V)
{
	VOutput Result = (VOutput)0;
	Result.Position = float4(V.Position, 1.0);
	Result.TexCoord = Result.Position;

	return Result;
}

float4 ps_main(VOutput V) : SV_TARGET0
{
	float2 TexCoord = GetTexCoord(V.TexCoord);
	float D1 = LDepthBuffer.SampleLevel(Sampler, TexCoord, 0).x;
	float D2 = GetDepth(TexCoord);

	[branch] if (D2 < D1 || D1 >= 1.0)
		return float4(GetOpaque(TexCoord, D2, 0.0), 1.0);

	float3 Position = GetPosition(TexCoord, D1);
	float3 Eye = normalize(Position - vb_Position);
	float4 Normal = LNormalBuffer.Sample(Sampler, TexCoord);
	
	Material Mat = Materials[Normal.w];
	float4 Diffuse = LDiffuseBuffer.Sample(Sampler, TexCoord);
	float A = min(1.0, (1.0 - Diffuse.w) + Mat.Transparency);
	float R = max(0.0, Mat.Roughness.x + LSurfaceBuffer.Sample(Sampler, TexCoord).x * Mat.Roughness.y - 0.25) / 0.75;
	float2 F = TexCoord + (Mat.Refraction * length(Normal.xyz) - Mat.Refraction * 1.2);
	return float4(Diffuse.xyz * GetCoverage(F, R * Mips) * A, 1.0);
};