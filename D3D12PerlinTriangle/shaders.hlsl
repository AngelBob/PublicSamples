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

//*********************************************************
// Background Shaders
// Draw the background as a single large triangle without a
// vertex buffer.
//*********************************************************
struct PSBackgroundInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

PSBackgroundInput VSMainBackground(in uint id : SV_VertexID)
{
    //See: https://web.archive.org/web/20140719063725/http://www.altdev.co/2011/08/08/interesting-vertex-shader-trick/
    PSBackgroundInput result;

    result.uv.x = (id == 1) ?  2.0 :  0.0;
    result.uv.y = (id == 2) ?  2.0 :  0.0;

    result.position = float4(result.uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 1.0, 1.0);

    return result;
}

float4 PSMainBackground(PSBackgroundInput input) : SV_TARGET
{
    return g_texture.Sample(g_sampler, input.uv);
}

//*********************************************************
// Foreground Shaders
// Draw the foreground as an object with vertex defined
// colors.
//*********************************************************
struct PSObjectInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR0;
};

PSObjectInput VSMainObject(float4 position : POSITION, float4 color : COLOR)
{
    PSObjectInput result;

    result.position = position;
    result.color = color;

    return result;
}

float4 PSMainObject(PSObjectInput input) : SV_TARGET
{
    return input.color;
}
