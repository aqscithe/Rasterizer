
#include <cstdio>
#include <cstring>
#include <cassert>
#include <math.h>
#include <iostream>

#include <imgui.h>

#include <common/maths.hpp>

#include "renderer_impl.hpp"

rdrImpl* rdrInit(float* colorBuffer32Bits, float* depthBuffer, int width, int height)
{
    rdrImpl* renderer = new rdrImpl();

    renderer->fb.colorBuffer = reinterpret_cast<float4*>(colorBuffer32Bits);
    renderer->fb.depthBuffer = depthBuffer;
    renderer->fb.width  = width;
    renderer->fb.height = height;

    renderer->viewport = Viewport{ 0, 0, width, height };

    renderer->uniforms.wireframe = false;
    renderer->uniforms.RGBInterpolation = false;
    renderer->uniforms.depthTest = true;
    renderer->uniforms.backfaceCulling = true;
    renderer->uniforms.phong = true;
    renderer->uniforms.alphaBlending = true;
    renderer->uniforms.alpha = 1.f;
    renderer->uniforms.lineColor = { 1.f, 1.f, 1.f, 1.f };

    renderer->uniforms.light.enabled = true;
    renderer->uniforms.light.attnEnabled = true;
    renderer->uniforms.light.minFullAttnDistance = 10.f;
    renderer->uniforms.light.position = { 2.f, 10.f, 4.f, 1.f };
    renderer->uniforms.light.ambient = { 0.f, 0.f, 0.f, 1.f };
    renderer->uniforms.light.diffuse = { 0.f, 0.f, 0.f, 1.f };
    renderer->uniforms.light.specular = { 0.f, 0.f, 0.f, 1.f };
    renderer->uniforms.light.attenuation = { 1.f, 1.f, 1.f };

    return renderer;
}

void rdrShutdown(rdrImpl* renderer)
{
    for (Texture texture : renderer->textures)
        delete texture.colors;
    delete renderer;
}

void rdrSetProjection(rdrImpl* renderer, float* projectionMatrix)
{
    memcpy(renderer->uniforms.proj.e, projectionMatrix, 16 * sizeof(float));
}

void rdrSetView(rdrImpl* renderer, float* viewMatrix)
{
    memcpy(renderer->uniforms.view.e, viewMatrix, 16 * sizeof(float));
}

void rdrSetModel(rdrImpl* renderer, float* modelMatrix)
{
    memcpy(renderer->uniforms.model.e, modelMatrix, 16 * sizeof(float));
}

void rdrSetViewport(rdrImpl* renderer, int x, int y, int width, int height)
{
    renderer->viewport = Viewport{ x, y, width, height };
}

void rdrSetUniformLight(rdrImpl* renderer, int index, rdrLight* light)
{
    memcpy(&renderer->uniforms.light, light, sizeof(light));
}

void rdrSetBackground(rdrImpl* renderer, float* bgColor)
{
    memcpy(&renderer->uniforms.bgColor, reinterpret_cast<float4*>(bgColor), sizeof(float4));
}

void rdrSetTexture(rdrImpl* renderer, float* colors32Bits, int width, int height)
{
    int size = (int)(colors32Bits)[0] ;
    renderer->textures.push_back( Texture{ (float*)malloc(size * sizeof(float)), width, height, (int)colors32Bits[1] });
    size_t index = renderer->textures.size() - 1;
    memcpy(renderer->textures.at(index).colors, &colors32Bits[2], size * sizeof(float));
}

void drawPixel(float4* colorBuffer, int width, int height, int x, int y, float4 color)
{
    // access violation error if I put 'y > height' instead of 'y >= height' when y is equal to height
    if (x < 0 || x > width || y < 0 || y >= height)
        return;
    colorBuffer[x + y * width] = color;
}

void drawLine(float4* colorBuffer, int width, int height, int x0, int y0, int x1, int y1, float4 color)
{
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2, e2;

    for (;;) {
        drawPixel(colorBuffer, width, height, x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}

void drawLine(const Framebuffer& fb, float2 p0, float2 p1, float4 color)
{
    drawLine(fb.colorBuffer, fb.width, fb.height, (int)roundf(p0.x), (int)roundf(p0.y), (int)roundf(p1.x), (int)roundf(p1.y), color);
}

float2 remap(float origFrom, float origTo, float targetFrom, float targetTo, float value)
{
    return { 0.f, 0.f };
}

float3 ndcToScreenCoords(float3 ndc, const Viewport& viewport)
{
    return { 
        ((ndc.x / 2.f) + 0.5f) * viewport.width, 
        (1.f - ((ndc.y / 2.f) + 0.5f)) * viewport.height,
        (-(1.f / ndc.z) + 1.f) / 2.f
    };
}

bool isOutside(float4 clipCoord)
{
    return clipCoord.w < 0.f 
        || clipCoord.x > clipCoord.w
        || clipCoord.x < -clipCoord.w
        || clipCoord.y > clipCoord.w
        || clipCoord.y < -clipCoord.w
        || clipCoord.z > clipCoord.w
        || clipCoord.z < -clipCoord.w;
}


bool isInsideTriangle(float2 pixelCoord, float3* screenCoords)
{
    float2 p0 = screenCoords[0].xy; float2 p1 = screenCoords[1].xy; float2 p2 = screenCoords[2].xy;

    auto s = p0.y * p2.x - p0.x * p2.y + (p2.y - p0.y) * pixelCoord.x + (p0.x - p2.x) * pixelCoord.y;
    auto t = p0.x * p1.y - p0.y * p1.x + (p0.y - p1.y) * pixelCoord.x + (p1.x - p0.x) * pixelCoord.y;

    if ((s < 0) != (t < 0))
        return false;

    auto A = -p1.y * p2.x + p0.y * (p2.x - p1.x) + p0.x * (p1.y - p2.y) + p1.x * p2.y;

    return A < 0 ?
        (s <= 0 && s + t >= A) :
        (s >= 0 && s + t <= A);
}

// using barycentric coordinates to find weighted average of the 3 screen coordinates in the triangle
bool getVerticesWeight(float3& w, float2 pixelCoord, float3 screenCoords[3])
{
    w.e[0] = (((screenCoords[1].y - screenCoords[2].y) * (pixelCoord.x - screenCoords[2].x)) +
        ((screenCoords[2].x - screenCoords[1].x) * (pixelCoord.y - screenCoords[2].y))) /
        (((screenCoords[1].y - screenCoords[2].y) * (screenCoords[0].x - screenCoords[2].x)) +
        ((screenCoords[2].x - screenCoords[1].x) * (screenCoords[0].y - screenCoords[2].y)));

    w.e[1] = (((screenCoords[2].y - screenCoords[0].y) * (pixelCoord.x - screenCoords[2].x)) +
        ((screenCoords[0].x - screenCoords[2].x) * (pixelCoord.y - screenCoords[2].y))) /
        (((screenCoords[1].y - screenCoords[2].y) * (screenCoords[0].x - screenCoords[2].x)) +
        ((screenCoords[2].x - screenCoords[1].x) * (screenCoords[0].y - screenCoords[2].y)));

    w.e[2] = 1.f - w.e[0] - w.e[1];

    return (w.e[0] >= 0.f) && 
        (w.e[1] >= 0.f) && 
        (w.e[2] >= 0.f) && 
        isInsideTriangle(pixelCoord, screenCoords);
}


float3 getReflection(float3 lightVec, float3 normal)
{
    return 2.f * maths::max(maths::dotProduct(lightVec, normal), 0.f) * normal - lightVec;
}

float getDiffuse(float3 lightVec, float3 normal)
{
    return maths::max(maths::dotProduct(lightVec, normal), 0.f);
}

float getSpecular(const float3& camPos, float3 lightVec, float3 normal)
{
    float3 reflection = getReflection(lightVec, normal);
    return maths::max(maths::dotProduct(reflection, camPos), 0.f);
}

float getAttenuation(const float3& lightVec, const Light& light)
{
    float distance = maths::magnitude(lightVec);
    if (distance == 0.f)
        return 1.f;
    return maths::clamp(0.f, 1.f, light.minFullAttnDistance / distance);
}

float3 ka = { 1.f, 1.f, 1.f }; // ambient constant - contained in materials
float3 kd = { 1.f, 1.f, 1.f }; // diffuse constant - contained in materials
float3 ks = { 1.f, 1.f, 1.f }; // specular constant - contained in materials

float3 getShadedColor(const float3& camPos, const Light& light, float3 position, float3 normal)
{
    if (!light.enabled)
        return { 0.f, 0.f, 0.f };

    float3 lightVec = maths::normalize(light.position.xyz - position);

    float3 diffuseColor = kd * getDiffuse(lightVec, maths::normalize(normal)) * light.diffuse.rgb;
    float3 ambientColor = ka * light.ambient.rgb;
    float3 specularColor = ks * getSpecular(camPos, lightVec, maths::normalize(normal)) * light.specular.rgb;

    if (light.attnEnabled)
    {
        float3 attenuationColor = getAttenuation(light.position.xyz - position, light) * light.attenuation;
        return attenuationColor * (ambientColor + diffuseColor + specularColor);
    }
    return ambientColor + diffuseColor + specularColor;

    
}

static void vertexShader(const Uniforms& uniforms, const Texture& texture, const float3& rgb, const rdrVertex& vertex, Varyings& out, 
    const float4& worldCoord4, const float4& normalWCoord4, const float3& camPos)
{

    float2 texel = {};
    if (!uniforms.wireframe)
    {
        if (uniforms.RGBInterpolation)
        {
            out.color = rgb;
        }
        else
        {
            // mapping colors on texture to pixels on the screen
            float* texColors = texture.colors;

            // Fix for poorly mapped uv textures
            // rare cases where the u or v is below 0 or greater than 1
            float u = vertex.u < 0.f ? 0.0001f : vertex.u > 1.f ? 0.9999f : vertex.u;
            float v = vertex.v < 0.f ? 0.0001f : vertex.v > 1.f ? 0.9999f : vertex.v;

            texel = { floorf(u * texture.width), floorf(v * texture.height) };

            int index = 4 * ((int)texel.y * texture.width + (int)texel.x);
            out.color = { static_cast<int>(texColors[index + 0]) / 255.f,
                static_cast<int>(texColors[index + 1]) / 255.f,
                static_cast<int>(texColors[index + 2]) / 255.f
            };
        }
        
        if (uniforms.phong)
        {
            out.worldCoords = worldCoord4.xyz;
            out.normalWCoords = normalWCoord4.xyz;
        }
        else
        {
            if (uniforms.light.enabled)
                out.color += getShadedColor(camPos, uniforms.light, worldCoord4.xyz, normalWCoord4.xyz);
        }
    }
}


static float4 pixelShader(const Uniforms& uniforms, const float2 pixel, const Varyings& in, const float3& camPos)
{
    if (uniforms.phong)
    {
        if (uniforms.light.enabled)  
            return { in.color + getShadedColor(camPos, uniforms.light, in.worldCoords, maths::normalize(in.normalWCoords)), uniforms.alpha };
    }

    return { in.color, uniforms.alpha };
}

Varyings interpolateVaryings(const Varyings* varyings, const float3& w)
{
    Varyings r = { 0.f, 0.f, 0.f };

    for (int i = 0; i < 3; ++i)
    {
        // interpolating colors, world coord & normals in order to find the weighted average based on
        // location within the triangle
        // improves accuracy colors when applied to pixels
        r.color.e[i] = ((w.e[0] * varyings[0].color.e[i]) + (w.e[1] * varyings[1].color.e[i]) + (w.e[2] * varyings[2].color.e[i]));
        r.worldCoords.e[i] = { w.e[0] * varyings[0].worldCoords.e[i] + w.e[1] * varyings[1].worldCoords.e[i] + w.e[2] * varyings[2].worldCoords.e[i] };
        r.normalWCoords.e[i] = { w.e[0] * varyings[0].normalWCoords.e[i] + w.e[1] * varyings[1].normalWCoords.e[i] + w.e[2] * varyings[2].normalWCoords.e[i] };
    }

    return r;
}

// determines what pixel to display based on value of pixel in depth buffer
// versus any new depth value at that same pixel
bool depthTest(Framebuffer& fb, float2 p, float z )
{
    if (p.x < 0.f || p.x >= fb.width || p.y < 0 || p.y >= fb.height)
        return false;

    int index = p.y * fb.width + p.x;
    if (z < fb.depthBuffer[index])
    {
        fb.depthBuffer[index] = z;
        return true;
    }
    return false;
}

float getDepth(float3 screenCoords[3], float3 w)
{
    return (w.e[0] * screenCoords[0].z) + (w.e[1] * screenCoords[1].z) + (w.e[2] * screenCoords[2].z);
}

float4 alphaBlending(float4 shadedColor, const float4& bg)
{
    float alpha = shadedColor.a + (bg.a * (1.f - shadedColor.a));
    float3 color = { 0.f, 0.f, 0.f };

    // assuming straight
    if (alpha != 0.f)
        color = ((shadedColor.rgb * shadedColor.a) + (bg.rgb * bg.a * (1.f - shadedColor.a))) / alpha;

    // assuming premultiplied
    //color = shadedColor.rgb + (bg.rgb * (1.f - shadedColor.a));
    return { color, alpha };
}

void pixelCalculations(const Varyings* varyings, const float3& w, const Uniforms& uniforms, Framebuffer& fb, float2 pixel, const float3& camPos)
{
    Varyings pixelVaryings = interpolateVaryings(varyings, w);
    float4 shadedColor = pixelShader(uniforms, pixel, pixelVaryings, camPos);
    if (uniforms.alphaBlending)
        drawPixel(fb.colorBuffer, fb.width, fb.height, (int)pixel.x, (int)pixel.y, alphaBlending(shadedColor, uniforms.bgColor));
    else
        drawPixel(fb.colorBuffer, fb.width, fb.height, (int)pixel.x, (int)pixel.y, shadedColor);
}

void rasterizeTriangle(Framebuffer& fb, const Uniforms& uniforms, float3 screenCoords[3], const Varyings* varyings, const float3& camPos)
{
    int minX = maths::min(maths::min((int)screenCoords[0].x, (int)screenCoords[1].x), (int)screenCoords[2].x);
    int maxX = maths::max(maths::max((int)screenCoords[0].x, (int)screenCoords[1].x), (int)screenCoords[2].x);
    int minY = maths::min(maths::min((int)screenCoords[0].y, (int)screenCoords[1].y), (int)screenCoords[2].y);
    int maxY = maths::max(maths::max((int)screenCoords[0].y, (int)screenCoords[1].y), (int)screenCoords[2].y);

    for (int y = minY; y <= maxY; ++y)
    {
        for (int x = minX; x <= maxX; ++x)
        {
            float2 pixel = { (float)x, (float)y };
            float3 w;
            if (getVerticesWeight(w, pixel, screenCoords))
            {
                if (uniforms.depthTest)
                {
                    if (depthTest(fb, pixel, getDepth(screenCoords, w)))
                    {
                        pixelCalculations(varyings, w, uniforms, fb, pixel, camPos);
                    }
                }
                else
                {
                    pixelCalculations(varyings, w, uniforms, fb, pixel, camPos);
                }
            }
        }
    }
}

bool isBackface(const Uniforms& uniforms, const rdrVertex& vertex, float4& worldCoord, float4& worldNormal, const float3& camPos)
{
    float3 localCoords = { vertex.x, vertex.y, vertex.z };
    worldCoord = uniforms.model * float4{ localCoords , 1.f };

    float3 localNormalCoords = { vertex.nx, vertex.ny, vertex.nz };
    worldNormal = uniforms.model * float4{ localNormalCoords , 0.f };

    if (uniforms.backfaceCulling && !uniforms.wireframe)
    {
        if (maths::dotProduct(maths::normalize(camPos - worldCoord.xyz), maths::normalize(worldNormal.xyz)) <= 0.f)
            return true;
    }
    return false;
}

float3 getCamPos(const mat4x4& view)
{
    mat4x4 inverted;
    if (mat4::invert(view.e, inverted.e))
    {
        return { inverted.c[3].e[0], inverted.c[3].e[1], inverted.c[3].e[2] };
    }
    else
    {
        std::cerr << "getCamPos() Function - View Matrix not invertible" << std::endl;
        exit(1);
    }
    return { 0.f, 0.f, 0.f };
}


void drawTriangle(rdrImpl* renderer, rdrVertex* vertices, int vertexIndex)
{
    Varyings varyings[3];

    int t = 0;
    int texCount = renderer->textures.size();
    int vCount = 0;

    for (int i = 0; i < texCount; ++i)
    {
        vCount += renderer->textures.at(i).vertexCount;
        if (vertexIndex <= vCount)
        {
            t = i;
            break;
        }
    }

    
    float3 camPos = {};
    // Getting the camera position is expensive because of the inverse matrix calculation
    // So I'm limiting how often the calculation is performed.
    // I'm also passing the camPos to the rasterizeTriangle() function to avoid needing
    // to perform the calculation again for specular lighting.
    if (!renderer->uniforms.wireframe && (renderer->uniforms.backfaceCulling || renderer->uniforms.light.enabled))
        camPos = getCamPos(renderer->uniforms.view);

    float4 worldCoord4[3];
    float4 worldNormal4[3];

    float4 clipCoords[3];
    float3 rgb[3] = { {1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f} };
    for (int i = 0; i < 3; ++i)
    {
        if (isBackface(renderer->uniforms, vertices[i], worldCoord4[i], worldNormal4[i], camPos))
            return;
        else
        {
            vertexShader(renderer->uniforms, renderer->textures[t], rgb[i], vertices[i], varyings[i], worldCoord4[i], worldNormal4[i], camPos);
            clipCoords[i] = renderer->uniforms.viewProj * worldCoord4[i];
        }
            
    }

    // clip triangles w/ vertices outside edge of screen
    for (int i = 0; i < 3; ++i)
    {
        if (isOutside(clipCoords[i]))  
            return;
    }

    // clip -> NDC
    // perspective divide in graphics pipeline
    float3 ndcCoords[3];
    for (int i = 0; i < 3; ++i)
    {
        ndcCoords[i] = clipCoords[i].xyz / clipCoords[i].w;
    }

    // NDC (v3) to screen coords (v2)
    float3 screenCoords[3];
    for (int i = 0; i < 3; ++i)
    {
        screenCoords[i] = ndcToScreenCoords(ndcCoords[i], renderer->viewport);
    }

    if (renderer->uniforms.wireframe)
    {
        for (int i = 0; i < 3; ++i)
        {
            drawLine(renderer->fb, screenCoords[i].xy, screenCoords[(i + 1) % 3].xy, renderer->uniforms.lineColor);
        }
    }
    else
    {
        rasterizeTriangle(renderer->fb, renderer->uniforms, screenCoords, varyings, camPos);
    }
}

void rdrDrawTriangles(rdrImpl* renderer, rdrVertex* vertices, int count)
{
    renderer->uniforms.modelViewProj = renderer->uniforms.proj * renderer->uniforms.view * renderer->uniforms.model;
    renderer->uniforms.viewProj = renderer->uniforms.proj * renderer->uniforms.view;

    // Transform vertex list to triangles into colorBuffer
    for (int i = 0; i < count; i += 3)
    {
        drawTriangle(renderer, &vertices[i], i + 1); 
    }
}

void rdrSetImGuiContext(rdrImpl* renderer, struct ImGuiContext* context)
{
    ImGui::SetCurrentContext(context);
}

void rdrShowImGuiControls(rdrImpl* renderer)
{
    ImGui::ColorEdit4("lineColor", renderer->uniforms.lineColor.e);
    ImGui::Checkbox("Wireframe", &renderer->uniforms.wireframe);
    ImGui::Checkbox("RGB Interpole", &renderer->uniforms.RGBInterpolation);
    ImGui::Checkbox("Depth Test", &renderer->uniforms.depthTest);
    ImGui::Checkbox("BF Culling", &renderer->uniforms.backfaceCulling);
    ImGui::Checkbox("Phong Shading", &renderer->uniforms.phong);
    ImGui::Checkbox("Alpha Blending", &renderer->uniforms.alphaBlending);
    ImGui::SliderFloat("Alpha Value", &renderer->uniforms.alpha, 0.f, 1.f);

    ImGui::Checkbox("Light Enabled", &renderer->uniforms.light.enabled);
    ImGui::Checkbox("Attenuation Enabled", &renderer->uniforms.light.attnEnabled);
    ImGui::DragFloat3("LightPos", renderer->uniforms.light.position.e);
    ImGui::DragFloat("Min Full Attn Distance ", &renderer->uniforms.light.minFullAttnDistance);
    ImGui::SliderFloat3("Attenuation Strength", renderer->uniforms.light.attenuation.e, 0.f, 1.f);
    ImGui::ColorEdit3("Ambient Color", renderer->uniforms.light.ambient.e);
    ImGui::ColorEdit3("Diffuse Color", renderer->uniforms.light.diffuse.e);
    ImGui::ColorEdit3("Specular Color", renderer->uniforms.light.specular.e);
}
