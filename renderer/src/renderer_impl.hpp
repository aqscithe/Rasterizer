#pragma once

#include <vector>

#include <rdr/renderer.h>

#include <common/types.hpp>

struct Viewport
{
    int x;
    int y;
    int width;
    int height;
};

struct Framebuffer
{
    int width;
    int height;
    float4* colorBuffer;
    float* depthBuffer;
};

struct Light
{
    bool enabled;
    bool attnEnabled;
    float minFullAttnDistance;
    float4 position; // World Space
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float3 attenuation;
};

struct Uniforms
{
    mat4x4 modelViewProj;
    mat4x4 viewProj;
    mat4x4 model;
    mat4x4 view;
    mat4x4 proj;

    bool wireframe;
    bool RGBInterpolation;
    bool depthTest;
    bool backfaceCulling;
    bool phong;
    bool alphaBlending;

    float alpha;
    float4 lineColor;
    float4 bgColor;

    // il faut créer un tableau de lumières
    //Light lights[3];
    Light light;
};

struct Varyings
{
    float3 color;
    float3 worldCoords;
    float3 normalWCoords;
};

struct Texture
{
    float* colors;
    int width;
    int height;
    int vertexCount;
};

struct rdrImpl
{
    Framebuffer fb;
    Viewport viewport;
    std::vector<Texture> textures;
    Uniforms uniforms;
};