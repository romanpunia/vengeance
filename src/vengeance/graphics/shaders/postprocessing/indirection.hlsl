#include "internal/layouts_shape.hlsl"
#include "internal/channels_effect.hlsl"
#include "internal/utils_material.hlsl"
#include "internal/utils_lighting.hlsl"
#include "internal/utils_raymarching.hlsl"
#include "internal/utils_random.hlsl"

Texture2D StochasticNormalMap : register(t5);
Texture2D EmissionMap : register(t6);

cbuffer RenderConstant : register(b3)
{
    float2 Random;
	float Samples;
	float Distance;
    float Initial;
    float Cutoff;
    float Attenuation;
    float Swing;
    float3 Padding;
    float Bias;
}

static const float3 RayCastingSphere[16] =
{
    float3(0.5381, 0.1856, -0.4319),
    float3(0.1379, 0.2486, 0.4430),
    float3(0.3371, 0.5679, -0.0057),
    float3(-0.6999, -0.0451, -0.0019),
    float3(0.0689, -0.1598, -0.8547),
    float3(0.0560, 0.0069, -0.1843),
    float3(-0.0146, 0.1402, 0.0762),
    float3(0.0100, -0.1924, -0.0344),
    float3(-0.3577, -0.5301, -0.4358),
    float3(-0.3169, 0.1063, 0.0158),
    float3(0.0103, -0.5869, 0.0046),
    float3(-0.0897, -0.4940, 0.3287),
    float3(0.7119, -0.0154, -0.0918),
    float3(-0.0533, 0.0596, -0.5411),
    float3(0.0352, -0.0631, 0.5460),
    float3(-0.4776, 0.2847, -0.0271)
};

float3 GetLightAt(float2 TexCoord)
{
    float Emission = SurfaceBuffer.SampleLevel(Sampler, TexCoord, 0).w;
    float3 Light = EmissionMap.SampleLevel(Sampler, TexCoord, 0).xyz;
    Light += Light * Emission;
    return Swing * pow(abs(Light), 1.0 / Attenuation);
}

VOutput vs_main(VInput V)
{
	VOutput Result = (VOutput)0;
	Result.Position = float4(V.Position, 1.0);
	Result.TexCoord = Result.Position;

	return Result;
}

float4 ps_main(VOutput V) : SV_TARGET0
{
    float2 UV = GetTexCoord(V.TexCoord);
    float3 Color = EmissionMap.SampleLevel(Sampler, UV, 0).xyz;
    if (Initial > 0.0)
        Color = float3(0.0, 0.0, 0.0);
    
	Fragment Frag = GetFragment(UV);
	[branch] if (Frag.Depth >= 1.0)
		return float4(Color.xyz, 1.0);

	Material Mat = Materials[Frag.Material];
	float3 Metallic = 1.0 - GetMetallic(Frag, Mat);
    float3 Accumulation = 0.0;
    float2 Jitter = RandomFloat2(UV + Random) + 0.5;
	float4 Normal = StochasticNormalMap.SampleLevel(Sampler, UV, 0);
    float Step = (1.0 / (float)Samples), Counter = 0.0;
    Step = Step * (Jitter.x + Jitter.y) + Step;

    for (float i = 0; i < Samples; i++)
    {
        float3 Direction = normalize(Normal.xyz + RayCastingSphere[i]);
        [branch] if (dot(Frag.Normal, Direction) < Cutoff)
            continue;

        float3 TexCoord = Raymarch(Frag.Position, Direction, Samples, Step * Distance);
        [branch] if (TexCoord.z == -1.0)
            continue;

        float3 Light = GetLightAt(TexCoord.xy);
        float Emission = distance(Frag.Position, GetPosition(TexCoord.xy, TexCoord.z)) / Distance;
        Emission = Bias * Emission * length(Light) / 3.0;
        Accumulation += Light * (1.0 - Emission * Emission);
        Counter++;
    }

    if (Counter > 0.0)
        Accumulation /= Counter;
    
    return float4(Color + Metallic * Accumulation, 1.0);
};