#include "internal/buffers_viewer.hlsl"
#include "internal/objects_material.hlsl"
#include "internal/objects_fragment.hlsl"
#include "internal/utils_position.hlsl"

StructuredBuffer<Material> Materials : register(t0);
Texture2D DiffuseBuffer : register(t1);
Texture2D NormalBuffer : register(t2);
Texture2D DepthBuffer : register(t3);
Texture2D SurfaceBuffer : register(t4);
SamplerState Sampler : register(s1);

bool IsInPixelGrid(float2 Texcoord)
{
	return Texcoord.x >= 0.0 && Texcoord.x <= 1.0 && Texcoord.y >= 0.0 && Texcoord.y <= 1.0;
}
float Linearize(float Depth)
{
	return vb_Near * vb_Far / (vb_Far + Depth * (vb_Near - vb_Far));
}
float GetDepth(float2 Texcoord)
{
	return DepthBuffer.SampleLevel(Sampler, Texcoord, 0).x;
}
float3 GetNormal(float2 Texcoord)
{
	return NormalBuffer.SampleLevel(Sampler, Texcoord, 0).xyz;
}
float4 GetDiffuse(float2 Texcoord, float Level)
{
	return DiffuseBuffer.SampleLevel(Sampler, Texcoord, Level);
}
float4 GetDiffuseSample(float2 Texcoord)
{
	return DiffuseBuffer.Sample(Sampler, Texcoord);
}
Fragment GetFragment(float2 Texcoord)
{
	float4 C0 = DiffuseBuffer.SampleLevel(Sampler, Texcoord, 0);
	float4 C1 = NormalBuffer.SampleLevel(Sampler, Texcoord, 0);
	float4 C2 = DepthBuffer.SampleLevel(Sampler, Texcoord, 0);
	float4 C3 = SurfaceBuffer.SampleLevel(Sampler, Texcoord, 0);
	float3 Position = GetPosition(Texcoord, C2.x);

	Fragment Result;
	Result.Position = Position;
	Result.Diffuse = C0.xyz;
	Result.Alpha = C0.w;
	Result.Normal = C1.xyz;
	Result.Material = C1.w;
	Result.Depth = C2.x;
	Result.Roughness = C3.x;
	Result.Metallic = C3.y;
	Result.Occlusion = C3.z;
	Result.Emission = C3.w;

	return Result;
}