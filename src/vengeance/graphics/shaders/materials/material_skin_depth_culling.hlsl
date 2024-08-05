#include "internal/layouts_skin.hlsl"
#include "internal/buffers_object.hlsl"
#include "internal/buffers_animation.hlsl"

VOutput vs_main(VInput V)
{
	VOutput Result = (VOutput)0;
	[branch] if (ab_Animated > 0)
	{
		matrix Offset =
			mul(ab_Offsets[(int)V.Index.x], V.Bias.x) +
			mul(ab_Offsets[(int)V.Index.y], V.Bias.y) +
			mul(ab_Offsets[(int)V.Index.z], V.Bias.z) +
			mul(ab_Offsets[(int)V.Index.w], V.Bias.w);

		Result.Position = mul(mul(float4(V.Position, 1.0), Offset), ob_Transform);
	}
	else
		Result.Position = mul(float4(V.Position, 1.0), ob_Transform);

	return Result;
}