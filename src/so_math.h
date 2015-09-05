/* so_math - v0.04

Changelog
=========
29. august 2015
    rename lerpf to mixf
    added vector mixf
    clamp and map
    scale(x, y, z)

21. august 2015
    scale
    lerpf

15. august 2015
    Added perspective matrix projection
    Fixed some math bugs

22. july 2015
    Added orthographic matrix projection.

*/

#ifndef _so_math_h_
#define _so_math_h_
#include "math.h"

#ifndef PI
#define PI 3.14159265359
#endif

#ifndef TWO_PI
#define TWO_PI 6.28318530718
#endif

#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))

// http://h14s.p5r.org/2012/09/0x5f3759df.html
float
_fast_inv_sqrt(float x)
{
    float xhalf = 0.5f * x;
    int i = *(int*)&x;          // Integer representation of float
    i = 0x5f3759df - (i >> 1);  // Initial guess
    x = *(float*)&i;            // Converted to floating point
    x = x*(1.5f-(xhalf*x*x));   // One round of Newton-Raphson's method
    return x;
}

union vec2
{
    vec2(float X, float Y) : x(X), y(Y) {}
    vec2() : x(0), y(0) {}
    struct
    {
        float x, y;
    };
    float data[2];
};

union vec3
{
    vec3(float X, float Y, float Z) : x(X), y(Y), z(Z) {}
    vec3() : x(0), y(0), z(0) {}
    struct
    {
        float x, y, z;
    };
    float data[3];
};

union vec4
{
    vec4(float X, float Y, float Z, float W) : x(X), y(Y), z(Z), w(W) {}
    vec4() : x(0), y(0), z(0), w(0) {}
    struct
    {
        float x, y, z, w;
    };
    float data[4];
};

union mat4
{
    struct
    {
        vec4 x, y, z, w;
    };
    float data[16];
};

///////////// vec2

vec2 operator *(vec2 a, vec2 b)  { return vec2(a.x * b.x, a.y * b.y); }
vec2 operator *(vec2 a, float s) { return vec2(a.x * s, a.y * s); }
vec2 operator +(vec2 a, vec2 b)  { return vec2(a.x + b.x, a.y + b.y); }
vec2 operator -(vec2 a, vec2 b)  { return vec2(a.x - b.x, a.y - b.y); }
vec2 operator /(vec2 a, vec2 b)  { return vec2(a.x / b.x, a.y / b.y); }
vec2 operator /(vec2 a, float s) { return a * (1.0f / s); }
vec2 operator -(vec2 a) { return vec2(-a.x, -a.y); }
vec2 &operator *=(vec2 &a, vec2 b)  { a = a * b; return a; }
vec2 &operator *=(vec2 &a, float s) { a = a * s; return a; }
vec2 &operator +=(vec2 &a, vec2 b)  { a = a + b; return a; }
vec2 &operator -=(vec2 &a, vec2 b)  { a = a - b; return a; }

///////////// vec3

vec3 operator *(vec3 a, vec3 b) {
    return vec3(a.x * b.x, a.y * b.y, a.z * b.z);
}
vec3 operator *(vec3 a, float s) {
    return vec3(a.x * s, a.y * s, a.z * s);
}
vec3 operator +(vec3 a, vec3 b) {
    return vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}
vec3 operator -(vec3 a, vec3 b) {
    return vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}
vec3 operator /(vec3 a, vec3 b) {
    return vec3(a.x / b.x, a.y / b.y, a.z / b.z);
}
vec3 operator /(vec3 a, float s) {
    return vec3(a.x / s, a.y / s, a.z / s);
}
vec3 &operator *=(vec3 &a, vec3 b)  { a = a * b; return a; }
vec3 &operator *=(vec3 &a, float s) { a = a * s; return a; }
vec3 &operator +=(vec3 &a, vec3 b)  { a = a + b; return a; }
vec3 &operator -=(vec3 &a, vec3 b)  { a = a - b; return a; }

///////////// vec4

vec4 operator *(vec4 a, vec4 b) {
    return vec4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}
vec4 operator *(vec4 a, float s) {
    return vec4(a.x * s, a.y * s, a.z * s, a.w * s);
}
vec4 operator +(vec4 a, vec4 b) {
    return vec4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
vec4 operator -(vec4 a, vec4 b) {
    return vec4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
vec4 operator /(vec4 a, vec4 b) {
    return vec4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}
vec4 operator /(vec4 a, float s) {
    return vec4(a.x / s, a.y / s, a.z / s, a.w / s);
}
vec4 &operator *=(vec4 &a, vec4 b)  { a = a * b; return a; }
vec4 &operator *=(vec4 &a, float s) { a = a * s; return a; }
vec4 &operator +=(vec4 &a, vec4 b)  { a = a + b; return a; }
vec4 &operator -=(vec4 &a, vec4 b)  { a = a - b; return a; }

///////////// mat4

vec4 operator *(mat4 a, vec4 b)
{
    vec4 result = a.x * b.x +
                  a.y * b.y +
                  a.z * b.z +
                  a.w * b.w;
    return result;
}

mat4 operator *(mat4 a, mat4 b)
{
    mat4 result = {};
    float *entry = result.data;
    for (int col = 0; col < 4; col++)
    for (int row = 0; row < 4; row++)
    {
        for (int i = 0; i < 4; i++)
            *entry += a.data[i * 4 + row] * b.data[col * 4 + i];
        entry++;
    }
    return result;
}

mat4
mat_identity()
{
    mat4 result = {};
    result.x.x = 1;
    result.y.y = 1;
    result.z.z = 1;
    result.w.w = 1;
    return result;
}

mat4
mat_scale(float s)
{
    mat4 result = {};
    result.x.x = s;
    result.y.y = s;
    result.z.z = s;
    result.w.w = 1;
    return result;
}

mat4
mat_scale(float x, float y, float z)
{
    mat4 result = {};
    result.x.x = x;
    result.y.y = y;
    result.z.z = z;
    result.w.w = 1;
    return result;
}

mat4
mat_rotate_x(float angle_in_radians)
{
    float c = cos(angle_in_radians);
    float s = sin(angle_in_radians);
    mat4 result = mat_identity();
    result.y.y = c;
    result.y.z = s;
    result.z.y = -s;
    result.z.z = c;
    return result;
}

mat4
mat_rotate_y(float angle_in_radians)
{
    float c = cos(angle_in_radians);
    float s = sin(angle_in_radians);
    mat4 result = mat_identity();
    result.x.x = c;
    result.x.z = s;
    result.z.x = -s;
    result.z.z = c;
    return result;
}

mat4
mat_rotate_z(float angle_in_radians)
{
    float c = cos(angle_in_radians);
    float s = sin(angle_in_radians);
    mat4 result = mat_identity();
    result.x.x = c;
    result.y.x = -s;
    result.x.y = s;
    result.y.y = c;
    return result;
}

mat4
mat_translate(vec3 x)
{
    mat4 result = mat_identity();
    result.w.x = x.x;
    result.w.y = x.y;
    result.w.z = x.z;
    return result;
}

mat4
mat_translate(float x, float y, float z)
{
    mat4 result = mat_identity();
    result.w.x = x;
    result.w.y = y;
    result.w.z = z;
    return result;
}

mat4
mat_ortho(float left, float right, float bottom, float top)
{
    mat4 result = mat_identity();
    result.x.x = 2.0f / (right - left);
    result.y.y = 2.0f / (top - bottom);
    return result;
}

mat4
mat_ortho_depth(float left, float right, float bottom, float top, float zn, float zf)
{
    mat4 result = mat_identity();
    result.x.x = 2.0f / (right - left);
    result.y.y = 2.0f / (top - bottom);
    result.z.z = 2.0f / (zn - zf);
    result.w.x = (right + left) / (left - right);
    result.w.y = (top + bottom) / (bottom - top);
    result.w.z = (zf + zn) / (zn - zf);
    return result;
}

mat4
mat_perspective(float fov, float width, float height, float zn, float zf)
{
    mat4 result = {};
    float a = width / height;
    result.x.x = 1.0f / (a * tan(fov / 2.0f));
    result.y.y = 1.0f / tan(fov / 2.0f);
    result.z.z = (zn + zf) / (zn - zf);
    result.z.w = -1.0f;
    result.w.z = 2.0f * zn * zf / (zn - zf);
    return result;
}

// vector functions

float
clampf(float x, float lo, float hi)
{
    if (x < lo)
        return lo;
    else if (x > hi)
        return hi;
    else
        return x;
}

float
mapf(float t0, float t1, float t, float y0, float y1)
{
    return clampf(y0 + (y1 - y0) * (t - t0) / (t1 - t0), y0, y1);
}

float mixf(float lo, float hi, float t) { return lo + (hi - lo) * t; }
vec2  mixf(vec2  lo, vec2  hi, float t) { return lo + (hi - lo) * t; }
vec3  mixf(vec3  lo, vec3  hi, float t) { return lo + (hi - lo) * t; }
vec4  mixf(vec4  lo, vec4  hi, float t) { return lo + (hi - lo) * t; }

float
dot(vec2 a, vec2 b)
{
    float result = a.x * b.x + a.y * b.y;
    return result;
}

float
dot(vec3 a, vec3 b)
{
    float result = a.x * b.x + a.y * b.y + a.z * b.z;
    return result;
}

float
dot(vec4 a, vec4 b)
{
    float result = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    return result;
}

float
length(vec2 a)
{
    float result = sqrt(dot(a, a));
    return result;
}

float
length(vec3 a)
{
    float result = sqrt(dot(a, a));
    return result;
}

float
length(vec4 a)
{
    float result = sqrt(dot(a, a));
    return result;
}

vec2
normalize(vec2 a)
{
    vec2 result = a * _fast_inv_sqrt(dot(a, a));
    return result;
}

vec3
normalize(vec3 a)
{
    vec3 result = a * _fast_inv_sqrt(dot(a, a));
    return result;
}

vec4
normalize(vec4 a)
{
    vec4 result = a * _fast_inv_sqrt(dot(a, a));
    return result;
}

#endif
