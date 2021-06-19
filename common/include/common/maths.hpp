#pragma once

#include <cmath>

#include "types.hpp"

// Constant and common maths functions
namespace maths
{
    const float TAU = 6.283185307179586476925f;

    template<typename T>
    inline int min(T x, T y) { return x < y ? x : y; };
    template<typename T>
    inline int max(T x, T y) { return x > y ? x : y; };

    inline float cos(float x) { return cosf(x); }
    inline float sin(float x) { return sinf(x); }
    inline float tan(float x) { return tanf(x); }

    inline float toRadians(float degrees) { return degrees * TAU / 360.f; };

    inline float dotProduct(float3 a, float3 b){ return a.x * b.x + a.y * b.y + a.z * b.z; }
    inline float magnitude(float3 v) { return sqrtf(dotProduct(v, v)); }
    float3 normalize(float3 v);
    inline float max(float a, float b) { return a > b ? a : b; }
    inline float min(float a, float b) { return a < b ? a : b; }
    inline float clamp(float min, float max, float val) { return val < min ? min : val > max ? max : val; }
}

namespace mat4
{
    mat4x4 identity();
    mat4x4 translate(float3 t);
    mat4x4 rotateX(float pitch);
    mat4x4 rotateY(float yaw);
    mat4x4 scale(float s);

    mat4x4 perspective(float fovY, float aspect, float near, float far);
    mat4x4 frustum(float left, float right, float bottom, float top, float near, float far);

    bool invert(const float in[16], float out[16]);

    //mat4x4 frustum(float left, float right, float bottom, float top, float near, float far);
    //mat4x4 perspective(float fovY, float aspect, float near, float far);
    //mat4x4 rotateY(float angleRadians);
}

inline float4 operator*(const mat4x4& m, float4 v)
{
    float4 r;
    r.x = v.x * m.c[0].e[0] + v.y * m.c[1].e[0] + v.z * m.c[2].e[0] + v.w * m.c[3].e[0];
    r.y = v.x * m.c[0].e[1] + v.y * m.c[1].e[1] + v.z * m.c[2].e[1] + v.w * m.c[3].e[1];
    r.z = v.x * m.c[0].e[2] + v.y * m.c[1].e[2] + v.z * m.c[2].e[2] + v.w * m.c[3].e[2];
    r.w = v.x * m.c[0].e[3] + v.y * m.c[1].e[3] + v.z * m.c[2].e[3] + v.w * m.c[3].e[3];
    return r;
}
//
inline mat4x4 operator*(const mat4x4& a, const mat4x4& b)
{
    mat4x4 res = {};
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r)
            for (int i = 0; i < 4; ++i)
                res.c[c].e[r] += a.c[i].e[r] * b.c[c].e[i];
    return res;
}

inline mat4x4& operator*=(mat4x4& a, const mat4x4& b)
{
    a = a * b;
    return a;
}

inline float3 operator+(float3 a, float3 b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }

inline float3 operator+=(float3& a, float3 b)
{
    a = a + b;
    return a;
}

inline float3 operator-(float3 a){return { -a.x, -a.y, -a.z };}
inline float3 operator-(float3 a, float3 b){return { a.x - b.x, a.y - b.y, a.z - b.z };}

inline float3 operator/(float3 v, float w){return { v.x / w, v.y / w, v.z / w };}

inline float3 operator*(float3 v, float k){return { v.x * k, v.y * k, v.z * k };}
inline float3 operator*(float k, float3 v){return { v.x * k, v.y * k, v.z * k };}
inline float3 operator*(float3 a, float3 b){return { a.x * b.x, a.y * b.y, a.z * b.z };}

inline float3 operator*=(float3& a, float k)
{
    a = a * k;
    return a;
}

inline float3 operator*=(float3& a, float3 b)
{
    a = a * b;
    return a;
}



//
//inline float3 operator/(float3 v, float a)
//{
//}
