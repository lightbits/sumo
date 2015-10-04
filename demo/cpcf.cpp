// http://github.prideout.net/coordinate-fields/

#include "sumo.h"
#include <stdio.h>
#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS
#define GLSL150(src) "#version 150\n" #src
#define CPCF_Width 512
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
out vec4 color;
void main()
{
    vec2 closest = texture(channel, texel).rg;
    float dist = length(texel - closest);
    float seperation = 0.05;
    float modulator = mod(dist / seperation, 1.0);
    float width = 0.005;
    float stripe = smoothstep(1.0 - width/seperation, 1.0, modulator) +
                   1.0 - smoothstep(0.0, width/seperation, modulator);
    color = vec4(0.08, 0.17, 0.36, 1.0);
    color = mix(color, 4.0*color, stripe);
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
    u32 i_min = 0;
    r32 d_min = length(p - seeds[i_min]);
    for (u32 i = 1; i < num_seeds; i++)
    {
        r32 d = length(p - seeds[i]);
        if (d < d_min)
        {
            i_min = i;
            d_min = d;
        }
    }
    return seeds[i_min];
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

    cpcf.seeds[0] = vec2(0.25f, 0.75f);
    cpcf.seeds[1] = vec2(0.75f, 0.25f);
    cpcf.seeds[2] = vec2(0.8f, 0.9f);

    compute_cpcf(cpcf.seeds, cpcf.field,
                 CPCF_Width, CPCF_Height, CPCF_Seeds);

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

    s32 xi = (s32)m_round(CPCF_Width * (io.mouse.pos.x / WINDOW_WIDTH));
    s32 yi = (s32)m_round(CPCF_Height * (io.mouse.pos.y / WINDOW_HEIGHT));
    if (xi < 0) xi = 0;
    if (xi >= CPCF_Width) xi = CPCF_Width - 1;
    if (yi < 0) yi = 0;
    if (yi >= CPCF_Height) yi = CPCF_Height - 1;
    s32 i = (yi * CPCF_Width + xi);
    vec2 q = cpcf.field[i];

    begin(&pass);
    uniformi("channel", 0);
    glBindTexture(GL_TEXTURE_2D, cpcf.tex);
    glBindBuffer(GL_ARRAY_BUFFER, quad);
    attribfv("position", 2, 2, 0);
    uniformi("channel", 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // ImGui::NewFrame();
    // persist float lightColor[4];
    // persist float attenuation;
    // const ImGuiWindowFlags layout_flags = ImGuiWindowFlags_NoTitleBar |
    //                                       ImGuiWindowFlags_NoResize   |
    //                                       ImGuiWindowFlags_NoMove;
    // ImGui::Begin("Main", 0, ImVec2(WINDOW_WIDTH, WINDOW_HEIGHT), 0.0f, layout_flags);
    // char text[255];
    // sprintf(text, "Pos: %d %d\nVal: %.2f %.2f", xi, yi, q.x, q.y);
    // ImGui::SetTooltip(text);
    // ImGui::End();
    // ImGui::Render();
}

#include "sumo.cpp"
