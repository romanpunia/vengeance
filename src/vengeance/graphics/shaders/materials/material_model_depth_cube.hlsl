#include "internal/layouts_vertex_instance.hlsl"
#include "internal/channels_depth.hlsl"
#include "internal/buffers_object.hlsl"
#include "internal/buffers_viewer.hlsl"
#include "internal/buffers_cube.hlsl"

[maxvertexcount(18)]
void gs_main(triangle VOutputLinear V[3], inout TriangleStream<VOutputCube> Stream)
{
	[unroll] for (uint Face = 0; Face < 6; Face++)
	{
		VOutputCube Result = (VOutputCube)0;
		Result.RenderTarget = Face;
		
		[unroll] for (uint Vertex = 0; Vertex < 3; Vertex++)
		{
			VOutputLinear Next = V[Vertex];
			Result.Position = mul(Next.Position, cb_ViewProjection[Face]);
			Result.UV = Next.UV;
			Result.Texcoord = Next.Texcoord;
			Stream.Append(Result);
		}

		Stream.RestartStrip();
	}
}

VOutputLinear vs_main(VInput V)
{
	VOutputLinear Result = (VOutputLinear)0;
	Result.Position = Result.UV = mul(float4(V.Position, 1.0), V.OB_World);
	Result.Texcoord = V.Texcoord * V.OB_Texcoord.xy;
    Result.OB_Diffuse = V.OB_Material.x;
    Result.OB_MaterialId = V.OB_Material.w;

	return Result;
}

float ps_main(VOutputLinear V) : SV_DEPTH
{
	float Threshold = (V.OB_Diffuse ? 1.0 - GetDiffuse(V.Texcoord).w : 1.0) * Materials[V.OB_MaterialId].Transparency;
	[branch] if (Threshold > 0.5)
		discard;
	
	return length(V.UV.xyz - vb_Position) / vb_Far;
};