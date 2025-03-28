#include "internal/layouts_shape.hlsl"
#include "internal/channels_geometry.hlsl"
#include "internal/buffers_object.hlsl"
#include "internal/buffers_viewer.hlsl"
#include "internal/utils_position.hlsl"

Texture2D LDepthBuffer : register(t8);

VOutput vs_main(VInput V)
{
	VOutput Result = (VOutput)0;
	Result.Position = mul(float4(V.Position, 1.0), ob_Transform);
	Result.Texcoord = Result.Position;

	return Result;
}

float4 ps_main(VOutput V) : SV_TARGET0
{
	float2 Texcoord = GetTexcoord(V.Texcoord);
	float Depth = LDepthBuffer.SampleLevel(Sampler, Texcoord, 0).x;
	[branch] if (Depth >= 1.0)
		discard;
	
	float4 Position = mul(float4(Texcoord.x * 2.0 - 1.0, 1.0 - Texcoord.y * 2.0, Depth, 1.0), ob_World);
	clip(0.5 - abs(Position.xyz));

	float4 Color = float4(Materials[ob_MaterialId].Diffuse, 1.0);
	[branch] if (ob_Diffuse > 0)
	{
		Color *= GetDiffuse(Texcoord * ob_Texcoord.xy);
		if (Color.w < 0.001)
			discard;
	}

	return Color;
};