// http://github.prideout.net/coordinate-fields/

// TODO: Investigate the effect of linearizing the distance field
// texture. e.g. can we render low res SDF and upscale with GL_LINEAR
// with good results?

// Using GL_LINEAR on the CPCF texture looks weird because you are
// "blending" two closest points.

// TODO: Easier way to debug draw RT textures
// TODO: Easier way to use RT and get color values at a given pixel
// from the RT

#include "sumo.h"
#include <stdio.h>
#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS
#define GLSL150(src) "#version 150\n" #src
#define CPCF_Width  512
#define CPCF_Height 512
#define CPCF_Seeds 3
#define global_var static
char *SDF_VS = GLSL150(
in vec2 position;
out vec2 texel;
void main()
{
    texel = vec2(0.5) + 0.5 * position;
    gl_Position = vec4(position, 0.0, 1.0);
}
);

char *SDF_FS = GLSL150(
in vec2 texel;
uniform sampler2D channel;
uniform int mode;
out vec4 color;
void main()
{
    if (mode == 0)
    {
        color = texture(channel, texel);
    }
    else if (mode == 1)
    {
        vec2 closest = texture(channel, texel).rg;
        vec2 p = texel;
        float dist = length(p - closest);
        float seperation = 0.2;
        float modulator = mod(dist / seperation, 1.0);
        float width = 0.005;
        float stripe = smoothstep(1.0 - width/seperation, 1.0, modulator) +
                       1.0 - smoothstep(0.0, width/seperation, modulator);
        color = vec4(1, 0.38, 0.22, 1.0) *
                vec4(0.5 + 0.5 * closest.x, 1.0,
                     0.5 + 0.5 * closest.y, 1.0);
        color = mix(color, 4.0*color, stripe);
    }
}
);

struct CPCF
{
    vec2 seeds[CPCF_Seeds];
    vec2 field[CPCF_Width*CPCF_Height];
    GLuint tex;
};

global_var RenderPass pass;
global_var GLuint quad;
global_var CPCF cpcf;

vec2 compute_d(vec2 *seeds, u32 num_seeds, vec2 p)
{
    vec2 s_min = seeds[0];
    r32 d_min = length(p - s_min);
    for (u32 i = 0; i < num_seeds; i++)
    {
        vec2 s = seeds[i];
        r32 d = length(p - s);
        if (d < d_min)
        {
            s_min = s;
            d_min = d;
        }

        // Each seed also has a circle of points, at
        // a radius of r away from the seed.
        r32 r = 0.2f;
        s = s + normalize(p - s) * r; // The closest point on the circle
        d = length(p - s);
        if (d < d_min)
        {
            s_min = s;
            d_min = d;
        }
    }

    return s_min;
}

void compute_cpcf(vec2 *seeds, vec2 *result,
                  u32 width, u32 height, u32 num_seeds)
{
    u32 i = 0;
    for (u32 y = 0; y < height; y++)
    for (u32 x = 0; x < width; x++)
    {
        vec2 p = vec2((r32)x / width, (r32)y / height);
        result[i++] = compute_d(seeds, num_seeds, p);
    }
}

void init()
{
    pass = make_render_pass(SDF_VS, SDF_FS);
    quad = make_quad();

    cpcf.seeds[0] = vec2(0.25f, 0.45f);
    cpcf.seeds[1] = vec2(0.5f, 0.5f);
    cpcf.seeds[2] = vec2(0.8f, 0.8f);
    compute_cpcf(cpcf.seeds, cpcf.field, CPCF_Width, CPCF_Height, CPCF_Seeds);
    cpcf.tex = so_make_tex2d(cpcf.field, CPCF_Width, CPCF_Height,
                             GL_RG32F, GL_RG, GL_FLOAT,
                             GL_NEAREST, GL_NEAREST,
                             GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
}

r32 m_floor(r32 x)
{
    return (r32)((s32)x);
}

r32 m_round(r32 x)
{
    return m_floor(x + 0.5f);
}

void tick(Input io, float t, float dt)
{
    clearc(0.35f, 0.55f, 1.0f, 1.0f);
    begin(&pass);
    uniformi("channel", 0);
    glBindTexture(GL_TEXTURE_2D, cpcf.tex);
    glBindBuffer(GL_ARRAY_BUFFER, quad);
    attribfv("position", 2, 2, 0);
    uniformi("channel", 0);
    if (io.key.down['a']) uniformi("mode", 1);
    else uniformi("mode", 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

#include "sumo.cpp"
