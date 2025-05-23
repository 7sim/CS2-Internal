#include "types.hlsli"

cbuffer sizeBuf : register(b0)
{
    float4 dimension;
}

cbuffer blurBuf : register(b2)
{
    float4 blur_amount_packed;
    float4 blur_weights_packed[24];
    float4 blur_offsets_packed[24];
}

static float blur_weights[96] = (float[96]) blur_weights_packed;
static float blur_offsets[96] = (float[96]) blur_offsets_packed;

// we could make this even faster by using the weighted offsets from https://www.rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
float3 blur(float2 pos, bool x_dir)
{
    float2 step = float2(1 / dimension.x, 1 / dimension.y);

    float3 col = rtCopyTex.Sample(rtCopySampler, pos).rgb * blur_weights[0];

    int it = int(blur_amount_packed.x);

    if (x_dir)
    {
		[loop]
        for (int i = 1; i < 25; ++i)
        {
            col += rtCopyTex.Sample(rtCopySampler, pos - float2(step.x * int(blur_offsets[i]), 0)).rgb * blur_weights[i];
            col += rtCopyTex.Sample(rtCopySampler, pos + float2(step.x * int(blur_offsets[i]), 0)).rgb * blur_weights[i];
        }
    }
    else
    {
		[loop]
        for (int i = 1; i < 25; ++i)
        {
            col += rtCopyTex.Sample(rtCopySampler, pos - float2(0, step.y * int(blur_offsets[i]))).rgb * blur_weights[i];
            col += rtCopyTex.Sample(rtCopySampler, pos + float2(0, step.y * int(blur_offsets[i]))).rgb * blur_weights[i];
        }
    }

    return col;
}