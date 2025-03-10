#include "internal/layouts_shape.hlsl"
#include "internal/channels_effect.hlsl"
#include "internal/utils_lighting.hlsl"
#include "internal/utils_material.hlsl"
#include "internal/utils_raymarching.hlsl"
#include "internal/utils_position.hlsl"

cbuffer RenderBuffer : register(b3)
{
	float Samples;
	float Padding;
	float Intensity;
	float Distance;
}

VOutput vs_main(VInput V)
{
	VOutput Result = (VOutput)0;
	Result.Position = float4(V.Position, 1.0);
	Result.Texcoord = Result.Position;

	return Result;
}

float4 ps_main(VOutput V) : SV_TARGET0
{
	Fragment Frag = GetFragment(GetTexcoord(V.Texcoord));
	[branch] if (Frag.Depth >= 1.0)
		return float4(0.0, 0.0, 0.0, 1.0);

	float3 Eye = normalize(Frag.Position - vb_Position);
	float3 Direction = reflect(Eye, Frag.Normal);
	float Fix = Rayprefix(Eye, Direction);
	[branch] if (Fix <= 0.0)
		return float4(0.0, 0.0, 0.0, 1.0);
	
	Material Mat = Materials[Frag.Material];
	float Fading = 1.0 - GetRoughnessMip(Frag, Mat, 1.0);
	float Step = Fading * Distance / Samples;

	[branch] if (Fading <= 0.0)
		return float4(0.0, 0.0, 0.0, 1.0);

	float3 Texcoord = Raymarch(Frag.Position, Direction, Samples, Step);
	[branch] if (Texcoord.z < 0.0)
		return float4(0.0, 0.0, 0.0, 1.0);

	float3 Metallic = GetMetallic(Frag, Mat);
	float3 Color = GetDiffuse(Texcoord.xy, 0).xyz * Intensity;
	Fix *= Raypostfix(Texcoord.xy, Direction);

	return float4(Metallic * Color * Fix, 1.0);
};