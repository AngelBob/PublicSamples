//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR0;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

PSInput VSMainBackground(float4 position : POSITION, float4 uv : TEXCOORD)
{
    PSInput result;

    result.position = position;
    result.uv = uv;

    return result;
}

float4 PSMainBackground(PSInput input) : SV_TARGET
{
    return g_texture.Sample(g_sampler, input.uv);
}

PSInput VSMainObject(float4 position : POSITION, float4 color : COLOR)
{
    PSInput result;

    result.position = position;
    result.color = color;

    return result;
}

float4 PSMainObject(PSInput input) : SV_TARGET
{
    return input.color;
}
