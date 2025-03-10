#include "internal/channels_geometry.hlsl"
#include "internal/objects_gbuffer.hlsl"

GBuffer Compose(float2 Texcoord, float4 Diffuse, float3 Normal, float Depth, float MaterialId)
{
	GBuffer Result;
	Result.DiffuseBuffer = Diffuse;
	Result.NormalBuffer.xyz = Normal;
	Result.NormalBuffer.w = MaterialId;
	Result.DepthBuffer = Depth;
	Result.SurfaceBuffer.x = RoughnessMap.Sample(Sampler, Texcoord).x;
	Result.SurfaceBuffer.y = MetallicMap.Sample(Sampler, Texcoord).x;
	Result.SurfaceBuffer.z = OcclusionMap.Sample(Sampler, Texcoord).x;
	Result.SurfaceBuffer.w = EmissionMap.Sample(Sampler, Texcoord).x;

	return Result;
}