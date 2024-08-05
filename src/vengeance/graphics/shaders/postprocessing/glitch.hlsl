#include "internal/layouts_shape.hlsl"
#include "internal/channels_effect.hlsl"
#include "internal/utils_random.hlsl"

cbuffer RenderConstant : register(b3)
{
	float ScanLineJitterDisplacement;
	float ScanLineJitterThreshold;
	float VerticalJumpAmount;
	float VerticalJumpTime;
	float ColorDriftAmount;
	float ColorDriftTime;
	float HorizontalShake;
	float ElapsedTime;
}

VOutput vs_main(VInput V)
{
	VOutput Result = (VOutput)0;
	Result.Position = float4(V.Position, 1.0);
	Result.TexCoord.xy = V.TexCoord;

	return Result;
}

float4 ps_main(VOutput V) : SV_TARGET0
{
	float Jitter = RandomFloatXY(V.TexCoord.y, ElapsedTime) * 2.0 - 1.0;
	Jitter *= step(ScanLineJitterThreshold, abs(Jitter)) * ScanLineJitterDisplacement;
	float Jump = lerp(V.TexCoord.y, frac(V.TexCoord.y + VerticalJumpTime), VerticalJumpAmount);
	float Shake = (RandomFloatXY(ElapsedTime, 2) - 0.5) * HorizontalShake;
	float Drift = sin(Jump + ColorDriftTime) * ColorDriftAmount;
	float4 Alpha = GetDiffuse(frac(float2(V.TexCoord.x + Jitter + Shake, Jump)), 0);
	float4 Beta = GetDiffuse(frac(float2(V.TexCoord.x + Jitter + Shake + Drift, Jump)), 0);

	return float4(Alpha.r, Beta.g, Alpha.b, 1);
};