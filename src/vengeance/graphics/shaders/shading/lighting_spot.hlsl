#include "internal/layouts_shape.hlsl"
#include "internal/channels_effect.hlsl"
#include "internal/utils_lighting.hlsl"
#include "internal/utils_material.hlsl"
#include "internal/utils_position.hlsl"
#include "internal/utils_multisample.hlsl"
#pragma warning(disable: 4000)

cbuffer SpotBuffer : register(b3)
{
	matrix LightTransform;
	matrix LightViewProjection;
	float4 Attenuation;
	float3 Direction;
	float Cutoff;
	float3 Position;
	float Range;
	float3 Lighting;
	float Softness;
	float Bias;
	float Iterations;
	float Umbra;
	float Padding;
};

#ifdef SHADOWED
Texture2D DepthMapLess : register(t5);
SamplerState DepthSampler : register(s5);
SamplerComparisonState DepthLessSampler : register(s6);
#endif

VOutput vs_main(VInput V)
{
	VOutput Result = (VOutput)0;
	Result.Position = mul(float4(V.Position, 1.0), LightTransform);
	Result.Texcoord = Result.Position;

	return Result;
}

float4 ps_main(VOutput V) : SV_TARGET0
{
	Fragment Frag = GetFragment(GetTexcoord(V.Texcoord));
	[branch] if (Frag.Depth >= 1.0)
		return float4(0, 0, 0, 0);

	float3 K = Position - Frag.Position;
	float3 L = normalize(K);
	float A = GetConeAttenuation(K, L, Attenuation.x, Attenuation.y, Range, Direction, Cutoff);
	[branch] if (A <= 0.0)
		return float4(0, 0, 0, 0);

	Material Mat = Materials[Frag.Material];
	float G = GetRoughness(Frag, Mat);
	float3 M = GetMetallic(Frag, Mat);
	float3 D = normalize(vb_Position - Frag.Position);
	float3 R = GetCookTorranceBRDF(Frag.Normal, D, L, Frag.Diffuse, M, G);
	float3 S = GetSubsurface(Frag.Normal, L, Mat.Subsurface, Mat.Scattering);
	R = Lighting * (R + S);
#ifdef SHADOWED
	float4 H = mul(float4(Frag.Position, 1), LightViewProjection);
#ifdef TARGET_D3D
	float2 T = float2(H.x / H.w / 2.0 + 0.5f, 1 - (H.y / H.w / 2.0 + 0.5f));
#else
	float2 T = float2(H.x / H.w / 2.0 + 0.5f, H.y / H.w / 2.0 + 0.5f);
#endif
	[branch] if (H.z > 0.0 && saturate(T.x) == T.x && saturate(T.y) == T.y)
	{
		float Q = 0.0, U = 0.0, Z = H.z / H.w - Bias;
		[loop] for (float j = 0; j < Iterations; j++)
		{
			float2 O = SampleDisk[j % 64].xy / Softness;
			float W = 1.0 - pow(abs(Frag.Depth - DepthMapLess.SampleLevel(DepthSampler, O + T, 0).x + Bias), Umbra);
			Q += DepthMapLess.SampleCmpLevelZero(DepthLessSampler, W * O + T, Z);
			U += W;
		}
		A *= lerp(Q / Iterations, 1.0, U / Iterations);
	}
#endif
	return float4(R * A, A);
};