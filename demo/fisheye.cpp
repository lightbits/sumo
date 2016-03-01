#include "sumo.h"
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS
#define GLSL(src) "#version 150\n" #src

/*
TODO: Correction based on measured height above ground?
lerp between parameters?
*/

char *vs = GLSL(
in vec2 position;
out vec2 v_position;
void main()
{
    v_position = position;
    gl_Position = vec4(position, 0.0, 1.0);
}
);

char *fs = GLSL(
in vec2 v_position;
uniform sampler2D channel0;
uniform float k1;
uniform float k2;
uniform float k3;
out vec4 f_color;
void main()
{
    vec2 vp = v_position;
    // vp.y *= -1280.0 / 1024.0;
    float rd = length(vp);

    float ru = k1 * atan(rd / k2 + rd * rd / k3);
    vp = normalize(vp) * ru;
    vec2 texel = vec2(0.5) + 0.5 * vp;
    if (texel.x < 0.0 || texel.x > 1.0 || texel.y < 0.0 || texel.y > 1.0)
        f_color = vec4(0.0);
    else
        f_color = texture(channel0, texel);
}
);

GLuint tex;
RenderPass pass;

void init()
{
    pass = make_render_pass(vs, fs);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void tick(Input io, float t, float dt)
{
    persist float k1 = 1.0f;
    persist float k2 = 1.0f;
    persist float k3 = 1.0f;
    persist int selected_frame = 1;
    persist int last_selected_frame = 0;

    glBindTexture(GL_TEXTURE_2D, tex);
    if (last_selected_frame != selected_frame)
    {
        last_selected_frame = selected_frame;
        char path[256];
        sprintf(path, "C:/Downloads/temp/video4/video%04d.png", selected_frame);
        s32 width, height, channels;
        u08 *data = stbi_load(path, &width, &height, &channels, 3);
        ASSERT(data);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                     width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }


    clearc(0.35f, 0.55f, 1.0f, 1.0f);
    begin(&pass);
    uniformi("channel0", 0);
    uniformf("k1", k1);
    uniformf("k2", k2);
    uniformf("k3", k3);
    so_draw_fullscreen_quad();

    ImGui::NewFrame();
    ImGui::SliderFloat("k1", &k1, -2.0f, 2.0f);
    ImGui::SliderFloat("k2", &k2, -0.5f, 0.5f);
    ImGui::SliderFloat("k3", &k3, -0.1f, 0.1f);
    ImGui::SliderInt("frame", &selected_frame, 1, 105);
    ImGui::Render();
}

#include "sumo.cpp"
