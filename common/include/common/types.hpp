#pragma once

union float2
{
    float e[2];
    struct { float x; float y; };
};

union float3
{
    float e[3];
    struct { float x; float y; float z; };
    struct { float r; float g; float b; };
    float2 xy;
};

union float4
{
    float4() = default;
    float4(float x, float y, float z, float w)
        : x(x), y(y), z(z), w(w)
    {}

    float4 (float3 xyz, float w)
        : x(xyz.x), y(xyz.y), z(xyz.z), w(w)
    {}

    float e[4];
    struct { float x; float y; float z; float w; };
    struct { float r; float g; float b; float a; };
    float3 rgb;
    float3 xyz;
    float2 xy;
};

union mat4x4
{
    float e[16];
    float4 c[4];
};
