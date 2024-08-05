#include "internal/layouts_shape.hlsl"
#include "internal/channels_effect.hlsl"
#include "internal/utils_lighting.hlsl"
#include "internal/utils_material.hlsl"
#include "internal/utils_position.hlsl"

cbuffer RenderConstant : register(b3)
{
	matrix LightTransform;
	float3 Position;
	float Range;
	float3 Lighting;
	float Mips;
	float3 Scale;
	float Parallax;
	float3 Attenuation;
	float Infinity;
};

TextureCube EnvironmentMap : register(t5);

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
	[branch] if (Mat.Environment <= 0.0)
		return float4(0.0, 0.0, 0.0, 0.0);

	float3 D = Position - Frag.Position;
	float3 E = normalize(Frag.Position - vb_Position);
	float3 M = GetMetallic(Frag, Mat);
	float R = GetRoughness(Frag, Mat);
	float A = max(Infinity, GetRangeAttenuation(D, Attenuation.x, Attenuation.y, Range)) * Mat.Environment;

	D = -normalize(reflect(-E, -Frag.Normal));
	[branch] if (Parallax > 0)
	{
		float3 Max = Position + Scale;
		float3 Min = Position - Scale;
		float3 Plane = ((D > 0.0 ? Max : Min) - Frag.Position) / D;

		D = Frag.Position + D * min(min(Plane.x, Plane.y), Plane.z) - Position;
	}
	
	float T = GetRoughnessMip(Frag, Mat, Mips);
#ifdef TARGET_D3D
	float3 P = EnvironmentMap.SampleLevel(Sampler, D, T).xyz;
#else
	float3 P = EnvironmentMap.SampleLevel(Sampler, float3(D.x, -D.y, D.z), T).xyz;
#endif
	float3 C = GetSpecularBRDF(Frag.Normal, -E, normalize(D), P, M, R);

	return float4(Lighting * C * A, A);
};