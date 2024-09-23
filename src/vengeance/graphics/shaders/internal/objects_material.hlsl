struct Material
{
	float4 Emission;
	float4 Metallic;
	float4 Subsurface;
	float3 Diffuse;
	float Fresnel;
	float3 Scattering;
	float Transparency;
	float3 Padding;
	float Bias;
	float2 Roughness;
	float Refraction;
	float Environment;
	float2 Occlusion;
	float Radius;
	float Height;
};