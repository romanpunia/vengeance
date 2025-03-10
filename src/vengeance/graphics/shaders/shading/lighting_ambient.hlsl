#include "internal/layouts_shape.hlsl"
#include "internal/channels_effect.hlsl"
#include "internal/utils_lighting.hlsl"
#include "internal/utils_material.hlsl"

cbuffer AmbientBuffer : register(b3)
{
	matrix SkyOffset;
	float3 HighEmission;
	float SkyEmission;
	float3 LowEmission;
	float LightEmission;
	float3 SkyColor;
	float FogFarOff;
	float3 FogColor;
	float FogNearOff;
	float3 FogFar;
	float FogAmount;
	float3 FogNear;
	float Recursive;
};

struct AmbientVertexResult
{
	float4 Position : SV_POSITION;
	float4 Texcoord : TEXCOORD0;
	float4 View : TEXCOORD1;
};

Texture2D LightMap : register(t5);
TextureCube SkyMap : register(t6);

float3 GetScattering(float3 Color, float Distance)
{
	float3 Far = float3(exp(-Distance * FogFar.x), exp(-Distance * FogFar.y), exp(-Distance * FogFar.z));
	float3 Near = float3(exp(-Distance * FogNear.x), exp(-Distance * FogNear.y), exp(-Distance * FogNear.z));
	float3 Result = FogColor * (1.0 - max(FogFarOff, Far)) + Color * max(FogNearOff, Near);
	return lerp(Color, Result, FogAmount);
}
float3 GetIllumination(float3 Up, float3 Down, float Height)
{
	return (Height * 0.5 + 0.5) * Up + Down;
}

AmbientVertexResult vs_main(VInput V)
{
	AmbientVertexResult Result = (AmbientVertexResult)0;
	Result.Position = float4(V.Position, 1.0);
	Result.Texcoord.xy = V.Texcoord;
	Result.View = mul(float4(V.Position.xy, 1.0, 1.0), SkyOffset);
	Result.View.xyz /= Result.View.w;

	return Result;
}

float4 ps_main(AmbientVertexResult V) : SV_TARGET0
{
	Fragment Frag = GetFragment(V.Texcoord.xy);
	float4 R = LightMap.SampleLevel(Sampler, V.Texcoord.xy, 0) * LightEmission;
	float L = distance(Frag.Position, vb_Position);

	[branch] if (Frag.Depth >= 1.0)
	{
		R.xyz = Frag.Diffuse + R.xyz * SkyColor * (1.0 - SkyEmission) + SkyMap.Sample(Sampler, V.View.xyz).xyz * SkyEmission;
		return float4(GetScattering(R.xyz, L), 1.0);
	}

	Material Mat = Materials[Frag.Material];
	float3 E = GetEmission(Frag, Mat);
	float3 O = GetOcclusion(Frag, Mat);
	float3 M = GetBaseReflectivity(Frag.Diffuse, GetMetallic(Frag, Mat));
	float3 A = GetIllumination(HighEmission, LowEmission, Frag.Normal.y);
	float3 I = GetIllumination(E, E * 0.5, Frag.Normal.y);
	float3 F = GetFresnelSchlick(dot(normalize(Frag.Position - vb_Position), Frag.Normal), M);
	float3 D = lerp(Frag.Diffuse.xyz, F, F * Mat.Fresnel * 0.05);
	R.xyz += max(A, I) * D * O;

	return float4(GetScattering(R.xyz, L), 1.0);
};