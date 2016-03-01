#include "sumo.h"
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS
#define GLSL(src) "#version 150\n" #src

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
uniform float height;
uniform float radius;
out vec4 f_color;
void main()
{
    vec2 vp = v_position*radius/sqrt(2.0);
    float h = sqrt(radius*radius-dot(vp, vp));
    vec3 ro = vec3(vp, h);
    vec3 rd = ro / radius;

    float t = (height - ro.z) / rd.z;
    vec3 p = ro + t * rd;

    vec2 texel = vec2(0.5) + 0.5 * p.xy * 0.1;
    if (texel.x < 0.0 || texel.x > 1.0 ||
        texel.y < 0.0 || texel.y > 1.0)
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

    persist float radius_mm = 3.5f;
    persist float height = 1.7f;
    float radius = radius_mm * 0.001f;

    clearc(0.35f, 0.55f, 1.0f, 1.0f);
    begin(&pass);
    uniformi("channel0", 0);
    uniformf("height", height);
    uniformf("radius", radius);
    so_draw_fullscreen_quad();

    ImGui::NewFrame();
    ImGui::SliderFloat("height", &height, 0.0f, 3.0f);
    ImGui::SliderFloat("radius", &radius_mm, 0.0f, 10.0f);
    ImGui::SliderInt("frame", &selected_frame, 1, 105);
    ImGui::Render();
}

#include "sumo.cpp"
