#version 150

in vec2 v_position;
uniform float aspect;
uniform int steps;
uniform sampler2D sampler0; // t | hit | UNUSED | UNUSED
out vec4 out0; // t | hit | UNUSED | UNUSED
out vec4 out1; // color r g b | UNUSED

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

float Terrain(vec3 p)
{
    float h = 0.0;

    h += 0.8*sin(p.x)*cos(p.z);

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

bool March(float t0, vec3 ro, vec3 rd, int steps, out vec3 out_p, out float out_t)
{
    #define T_MAX 128.0
    #define STEP_SIZE 0.03
    float t = t0;
    vec3 p0 = ro + t * rd;
    float f0 = Terrain(p0);
    for (int i = 0; i < steps; i++)
    {
        // TODO: Sketchy here?, need to incr t?
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
                out_t = t;
                return true;
            }
            t = length(p1 - ro);
        }
        p0 = p1;
        f0 = f1;
        t += STEP_SIZE;
        if (t > T_MAX)
            break;
    }
    out_t = t - STEP_SIZE;
    return false;
}

float Lighting(vec3 p, vec3 N, vec3 L)
{
    float NdotL = max(dot(N, L), 0.0);
    return NdotL;
    // vec3 UNUSED_p;
    // float UNUSED_t;
    // if (March(0.0, p + 2.0*0.001*N, L, UNUSED_p, UNUSED_t))
    // {
    //     return 0.0;
    // }
    // else
    // {
    //     // return 1.0;
    //     float NdotL = max(dot(N, L), 0.0);
    //     return NdotL;
    // }
}

vec3 SkyColor(vec3 rd)
{
    vec3 sky = vec3(0.27, 0.5, 0.9);
    vec3 horizon = mix(sky, vec3(1.1, 1.05, 0.9), 0.7);
    return mix(horizon, sky, 1.2*smoothstep(0.0, 2.0, 3.5*rd.y));
}

vec3 Shade(vec3 p, vec3 rd)
{
    vec3 L1 = normalize(vec3(1.33, 0.4, 1.5));
    vec3 L2 = normalize(vec3(-1.0, 0.4, -1.0));
    vec3 N = TerrainNormal(p);

    vec3 albedo = vec3(1.2, 0.9, 0.5);
    float h = p.y;
    h += 0.07*snoise(0.67*p.xz);
    h += 0.06*snoise(2.88*p.xz);
    h += 0.03*snoise(8.5*p.xz);
    h += 0.03*snoise(32.5*p.xz);
    albedo = mix(albedo, vec3(10.0, 9.0, 8.0), smoothstep(-0.1, 0.5, h));

    vec3 color = vec3(0.0);
    color += Lighting(p, N, L1) * albedo;
    color += 0.1*Lighting(p, N, L2) * albedo;

    float fog = 1.0 - exp(-0.005*dot(p, p));
    color = mix(color, 0.8*SkyColor(rd), fog);
    return color;
}

void main()
{
    vec3 ro = vec3(0.6, 2.5, 0.0);
    vec3 right = vec3(1.0, 0.0, 0.0) * aspect;
    vec3 up = normalize(vec3(0.0, 1.0, 0.0));
    vec3 forward = normalize(cross(right, up));
    vec3 rd = normalize(forward*1.2 + v_position.x * right + v_position.y * up);

    vec2 texel = vec2(0.5) + 0.5 * v_position;
    vec4 in0 = texture(sampler0, texel);
    float t0 = in0.x;
    float hit0 = in0.y;
    if (hit0 < 0.5) // no hit yet, take a step
    {
        float t;
        vec3 p;
        if (March(t0, ro, rd, steps, p, t))
        {
            out0.x = t;
            out0.y = 1.0;
            out1.rgb = vec3(1.0);
            out1.a = 1.0;
        }
        else
        {
            out0.x = t;
            out0.y = 0.0;
            out1.rgb = SkyColor(rd);
            out1.a = 0.0;
        }
    }
    else
    {
        out0.x = t0;
        out0.y = 1.0;
        out1.rgb = Shade(ro + rd * t0, rd);
        out1.a = 1.0;
    }

    out1.rgb = pow(out1.rgb, vec3(0.4545));
    out1.rgb = smoothstep(vec3(0.1), vec3(1.2), out1.rgb);
}
