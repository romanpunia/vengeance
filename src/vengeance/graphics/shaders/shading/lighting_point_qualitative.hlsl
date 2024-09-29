#include "internal/layouts_shape.hlsl"
#include "internal/channels_effect.hlsl"
#include "internal/utils_lighting.hlsl"
#include "internal/utils_material.hlsl"
#include "internal/utils_position.hlsl"
#include "internal/utils_multisample.hlsl"
#include "shading/internal/lighting_point_buffer.hlsl"
#pragma warning(disable: 4000)

TextureCube DepthMapLess : register(t5);
SamplerState DepthSampler : register(s5);
SamplerComparisonState DepthLessSampler : register(s6);

VOutput vs_main(VInput V)
{
	VOutput Result = (VOutput)0;
	Result.Position = mul(float4(V.Position, 1.0), LightTransform);
	Result.TexCoord = Result.Position;

	return Result;
}

float4 ps_main(VOutput V) : SV_TARGET0
{
	Fragment Frag = GetFragment(GetTexCoord(V.TexCoord));
	[branch] if (Frag.Depth >= 1.0)
		return float4(0, 0, 0, 0);

	Material Mat = Materials[Frag.Material];
	float G = GetRoughness(Frag, Mat);
	float3 M = GetMetallic(Frag, Mat);
	float3 K = Position - Frag.Position;
	float3 D = normalize(vb_Position - Frag.Position);
	float3 L = normalize(K);
	float3 R = GetCookTorranceBRDF(Frag.Normal, D, L, Frag.Diffuse, M, G);
	float3 S = GetSubsurface(Frag.Normal, L, Mat.Subsurface, Mat.Scattering);
	float A = GetRangeAttenuation(K, Attenuation.x, Attenuation.y, Range);
#ifndef TARGET_D3D
	L.y = -L.y;
#endif
	float Q = 0.0, U = 0.0, Z = length(K) / Distance - Bias;
	[loop] for (float j = 0; j < Iterations; j++)
	{
		float3 O = SampleDisk[j % 64] / Softness;
		float W = 1.0 - pow(abs(Frag.Depth - DepthMapLess.SampleLevel(DepthSampler, O - L, 0).x), Umbra);
		Q += DepthMapLess.SampleCmpLevelZero(DepthLessSampler, W * O - L, Z);
		U += W;
	}

	A *= lerp(Q / Iterations, 1.0, U / Iterations);
	return float4(Lighting * (R + S) * A, A);
};