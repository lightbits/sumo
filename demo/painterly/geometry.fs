#version 330
in vec2 v_texel;
uniform float aspect;
uniform vec3 origin;
uniform vec3 framez;
uniform vec3 framey;
uniform vec3 framex;
uniform float time;
layout(location = 0) out vec4 out0;
layout(location = 1) out vec4 out1;
layout(location = 2) out vec4 out2;

//
// Description : Array and textureless GLSL 2D simplex noise function.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//

vec3 mod289(vec3 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289(vec2 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x) {
    return mod289(((x*34.0)+1.0)*x);
}

float snoise(vec2 v)
{
    const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                        0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                       -0.577350269189626,  // -1.0 + 2.0 * C.x
                        0.024390243902439); // 1.0 / 41.0
    // First corner
    vec2 i  = floor(v + dot(v, C.yy) );
    vec2 x0 = v -   i + dot(i, C.xx);

    // Other corners
    vec2 i1;
    //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
    //i1.y = 1.0 - i1.x;
    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    // x0 = x0 - 0.0 + 0.0 * C.xx ;
    // x1 = x0 - i1 + 1.0 * C.xx ;
    // x2 = x0 - 1.0 + 2.0 * C.xx ;
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;

    // Permutations
    i = mod289(i); // Avoid truncation effects in permutation
    vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
          + i.x + vec3(0.0, i1.x, 1.0 ));

    vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
    m = m*m ;
    m = m*m ;

    // Gradients: 41 points uniformly over a line, mapped onto a diamond.
    // The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;

    // Normalise gradients implicitly by scaling m
    // Approximation of: m *= inversesqrt( a0*a0 + h*h );
    m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

    // Compute final noise value at P
    vec3 g;
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}


float SPHERE(vec3 p, float r)
{
    return length(p) - r;
}

// By Inigo Quilez
float BOX(vec3 p, vec3 b)
{
    vec3 d = abs(p) - b;
    return min(max(d.x,max(d.y,d.z)),0.0) +
         length(max(d,0.0));
}

#define UNITE(EXPR, ID) d1 = EXPR; if (d1 < d) { d = d1; id = ID; }
#define SUBTRACT(EXPR, ID) d1 = EXPR; if (-d1 > d) { d = -d1; id = ID; }

#if 1
#define STEPS 32
#define EPSILON 0.01
float MAP(vec3 p, out int id)
{
    float d1;
    float d = BOX(p, vec3(0.5));
    id = 0;
    SUBTRACT(SPHERE(p, 0.6 + 0.1*sin(0.01*time)), 0);
    UNITE(SPHERE(p, 0.3), 1);
    UNITE(SPHERE(p - vec3(1.0, 0.0, 0.0), 0.5), 2);
    SUBTRACT(SPHERE(p - vec3(0.7, 0.0, 0.5), 0.3), 1);
    UNITE(SPHERE(p - vec3(-1.0, 0.0, 0.0), 0.5), 0);
    return d;
}

float MAPN(vec3 p)
{
    int id;
    return MAP(p, id);
}

vec3 NORMAL(vec3 p)
{
    vec2 e = vec2(EPSILON, 0.0);
    return normalize(vec3(
                     MAPN(p + e.xyy) - MAPN(p - e.xyy),
                     MAPN(p + e.yxy) - MAPN(p - e.yxy),
                     MAPN(p + e.yyx) - MAPN(p - e.yyx)));
}

bool MARCH(vec3 ro, vec3 rd, out vec3 out_p, out int out_id)
{
    float t = 0.0;
    for (int i = 0; i < STEPS; i++)
    {
        vec3 p = ro + rd * t;
        float d = MAP(p, out_id);
        if (d < EPSILON)
        {
            out_p = p;
            return true;
        }
        t += d;
    }
    return false;
}
#else
float Terrain(vec3 p)
{
    float h = 0.0;

    float h0 = 0.5 + 3.5*smoothstep(8.0, 16.0, length(p));
    float f0 = 0.22 - 0.15*smoothstep(8.0, 24.0, length(p));
    h += h0*snoise(f0*p.xz);
    p.z += 0.1*snoise(0.88*p.xz);
    p.x += 0.1*snoise(0.93*p.xz);
    h += 0.05*snoise(0.88*p.xz);

    h += 0.001*snoise(20.0*p.xz);
    p.z += 0.1*snoise(1.18*p.xz);
    p.x += 0.1*snoise(1.13*p.xz);
    h += 0.002*snoise(8.13*p.xz);
    h += 0.01*snoise(2.13*p.xz);
    h += 0.005*snoise(4.13*p.xz);

    return p.y - h;
}

vec3 TerrainNormal(vec3 p)
{
    vec2 eps = vec2(0.001, 0.0);
    return normalize(vec3(
                     Terrain(p + eps.xyy) - Terrain(p - eps.xyy),
                     Terrain(p + eps.yxy) - Terrain(p - eps.yxy),
                     Terrain(p + eps.yyx) - Terrain(p - eps.yyx)));
}

bool March(vec3 ro, vec3 rd, out vec3 out_p)
{
    #define STEPS 512
    #define T_MAX 128.0
    #define STEP_SIZE 0.03
    float multiplier = 1.0;
    float t = 0.0;
    vec3 p0 = ro;
    float f0 = Terrain(p0);
    for (int i = 0; i < STEPS; i++)
    {
        vec3 p1 = ro + t * rd;
        float f1 = Terrain(p1);
        if (f0 > 0.0 && f1 < 0.0)
        {
            float l = -f0 / (f1 - f0);
            p1 = p0 + (p1 - p0) * l;
            f1 = Terrain(p1);
            if (f1 <= 0.1)
            {
                out_p = p1;
                return true;
            }
            t = length(p1 - ro);
        }
        p0 = p1;
        f0 = f1;
        t += STEP_SIZE * multiplier;
        multiplier += STEP_SIZE / 1.33;
        if (t > T_MAX)
            break;
    }
    return false;
}

float Lighting(vec3 p, vec3 N, vec3 L)
{
    // float NdotL = max(dot(N, L), 0.0);
    // return NdotL;
    vec3 t;
    if (March(p + 2.0*0.001*N, L, t))
    {
        return 0.0;
    }
    else
    {
        // return 1.0;
        float NdotL = max(dot(N, L), 0.0);
        return NdotL;
    }
}

vec3 SkyColor(vec3 rd)
{
    vec3 sky = vec3(0.27, 0.5, 0.9);
    vec3 horizon = mix(sky, vec3(1.1, 1.05, 0.9), 0.7);
    return mix(horizon, sky, 1.2*smoothstep(0.0, 2.0, 3.5*rd.y));
    // float h = 0.4 - 0.18*smoothstep(0.0, 1.0, 0.9*abs(rd.x-0.5));
    // vec3 color = mix(horizon, sky, smoothstep(0.0, h, rd.y));
    // color = mix(color, sky, smoothstep(0.0, 1.0, 0.9*abs(rd.x-0.5)));
    // return color;
}
#endif

float random(vec2 co)
{
    co += vec2(0.53*time, 0.701*time);
    return fract(sin(dot(co.xy,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    vec3 ro = origin;
    vec2 texel = v_texel;
    texel.x += random(v_texel+vec2(23.33, 18.7)) * 0.05;
    texel.y += random(v_texel) * 0.05;
    vec3 rd = normalize(framez*1.4 + texel.x*aspect*framex + texel.y*framey);

    vec3 offset = vec3(0.0);
    vec3 albedo = vec3(1.0);
    vec3 normal = vec3(0.0);

    #if 1
    vec3 p;
    int id;
    if (MARCH(ro, rd, p, id))
    {
        offset = p;
        normal = NORMAL(p);
        if (id == 0)
            albedo = vec3(1.0, 0.4, 0.4);
        if (id == 1)
            albedo = vec3(1.0, 1.0, 0.8);
        if (id == 2)
            albedo = vec3(0.5, 0.7, 1.0);
        out0.w = 1.0;
    }
    else
    {
        out0.w = 0.0;
    }
    #else
    vec3 p;
    if (March(ro, rd, p))
    {
        vec3 L1 = normalize(vec3(1.33, 0.4, 1.5));
        vec3 L2 = normalize(vec3(-1.0, 0.4, -1.0));
        vec3 N = TerrainNormal(p);

        vec3 color = vec3(0.0);
        albedo = vec3(1.2, 0.9, 0.5);
        float h = p.y;
        h += 0.07*snoise(0.67*p.xz);
        h += 0.06*snoise(2.88*p.xz);
        h += 0.03*snoise(8.5*p.xz);
        h += 0.03*snoise(32.5*p.xz);
        albedo = mix(albedo, vec3(10.0, 9.0, 8.0), smoothstep(-0.1, 0.5, h));

        color += Lighting(p, N, L1) * albedo;
        color += 0.1*Lighting(p, N, L2) * albedo;

        float fog = 1.0 - exp(-0.005*dot(p, p));
        color = mix(color, 0.8*SkyColor(rd), fog);

        albedo = color;
        normal = N;
        offset = p;
        out0.w = 1.0;
    }
    else
    {
        normal = vec3(0.0, 0.0, 1.0);
        offset = ro + 32.0 * rd;
        albedo = SkyColor(rd);
        out0.w = 0.0;
    }
    #endif

    out0.xyz = offset;
    out1.xyz = albedo;
    out2.xyz = normal;
}
