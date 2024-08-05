#include "internal/layouts_vertex.hlsl"
#include "internal/buffers_object.hlsl"

VOutput vs_main(VInput V)
{
	VOutput Result = (VOutput)0;
	Result.Position = mul(float4(V.Position, 1.0), ob_Transform);

	return Result;
}