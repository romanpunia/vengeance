#include "internal/layouts_shape.hlsl"
#include "internal/channels_effect.hlsl"
#include "internal/utils_lighting.hlsl"
#include "internal/utils_atmosphere.hlsl"
#include "internal/utils_material.hlsl"
#include "internal/utils_position.hlsl"
#include "internal/utils_multisample.hlsl"
#pragma warning(disable: 3595)

cbuffer LineBuffer : register(b3)
{
	matrix LightViewProjection[6];
	matrix SkyOffset;
	float3 RlhEmission;
	float RlhHeight;
	float3 MieEmission;
	float MieHeight;
	float3 Lighting;
	float Softness;
	float3 Position;
	float Cascades;
	float Padding;
	float Umbra;
	float Bias;
	float Iterations;
	float ScatterIntensity;
	float PlanetRadius;
	float AtmosphereRadius;
	float MieDirection;
};

#ifdef SHADOWED
Texture2D DepthMap[6] : register(t5);
SamplerComparisonState DepthLessSampler : register(s5);

float GetCascade(float3 Position, uniform uint Index)
{
	[branch] if (Index >= (uint)Cascades)
		return 1.0;

	float4 L = mul(float4(Position, 1), LightViewProjection[Index]);
#ifdef TARGET_D3D
	float2 T = float2(L.x / L.w / 2.0 + 0.5f, 1 - (L.y / L.w / 2.0 + 0.5f));
#else
	float2 T = float2(L.x / L.w / 2.0 + 0.5f, L.y / L.w / 2.0 + 0.5f);
#endif
	[branch] if (saturate(T.x) != T.x || saturate(T.y) != T.y)
		return -1.0;
	
	float Q = 0.0, Z = L.z / L.w - Bias;
	[loop] for (float j = 0; j < Iterations; j++)
	{
		float2 O = SampleDisk[j % 64].xy / Softness;
		Q += DepthMap[Index].SampleCmpLevelZero(DepthLessSampler, O + T, Z);
	}
	return Q / Iterations;
}
#endif

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
	Fragment Frag = GetFragment(Texcoord);
	[branch] if (Frag.Depth >= 1.0)
	{
		[branch] if (ScatterIntensity <= 0.0)
			return float4(0, 0, 0, 0);
			
		Scatter A;
		A.Sun = ScatterIntensity;
		A.Planet = PlanetRadius;
		A.Atmos = AtmosphereRadius;
		A.Rlh =  RlhEmission;
		A.Mie = MieEmission;
		A.RlhHeight = RlhHeight;
		A.MieHeight = MieHeight;
		A.MieG = MieDirection;
		return float4(GetAtmosphere(Texcoord, SkyOffset, float3(0, 6372e3, 0), float3(-Position.x, Position.y, Position.z), A), 1.0);
	}
	
	Material Mat = Materials[Frag.Material];
	float G = GetRoughness(Frag, Mat);
	float3 M = GetMetallic(Frag, Mat);
	float3 D = normalize(vb_Position - Frag.Position);
	float3 R = GetCookTorranceBRDF(Frag.Normal, D, Position, Frag.Diffuse, M, G);
	float3 S = GetSubsurface(Frag.Normal, Position, Mat.Subsurface, Mat.Scattering);
	R = Lighting * (R + S);
#ifdef SHADOWED
	float H = GetCascade(Frag.Position, 0);
	[branch] if (H >= 0.0)
		return float4(R * (H + S), 1.0);
		
	H = GetCascade(Frag.Position, 1);
	[branch] if (H >= 0.0)
		return float4(R * (H + S), 1.0);
		
	H = GetCascade(Frag.Position, 2);
	[branch] if (H >= 0.0)
		return float4(R * (H + S), 1.0);
		
	H = GetCascade(Frag.Position, 3);
	[branch] if (H >= 0.0)
		return float4(R * (H + S), 1.0);
		
	H = GetCascade(Frag.Position, 4);
	[branch] if (H >= 0.0)
		return float4(R * (H + S), 1.0);
		
	H = GetCascade(Frag.Position, 5);
	[branch] if (H >= 0.0)
		return float4(R * (H + S), 1.0);
#endif
	return float4(R, 1.0);
};