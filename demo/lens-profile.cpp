#define USE_NEW_MATH
#include "sumo.h"
#define WINDOW_WIDTH 1000
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
in vec2 texel;
uniform float scale;
uniform float aspect;
out vec2 v_texel;
void main()
{
    v_texel = texel;
    gl_Position = vec4(vec2(1.0, aspect) * position * scale, 0.0, 1.0);
}
);

char *fs = GLSL(
in vec2 v_texel;
uniform sampler2D channel0;
uniform float wireframe;
out vec4 f_color;
void main()
{
    f_color = texture(channel0, v_texel);
    f_color = mix(f_color, vec4(1.0, 1.0, 1.0, 0.02), wireframe);
}
);

GLuint tex;
GLuint mesh_ibo;
GLuint mesh_vbo;
RenderPass pass;

#define GRID_X 64
#define GRID_Y 64
struct Vertex
{
    vec2 position;
    vec2 texel;
};
Vertex vertices[GRID_X+1][GRID_Y+1];

void init()
{
    pass = make_render_pass(vs, fs);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    {
        float lens_radius_mm = 5.5f;
        float lens_radius = lens_radius_mm * 0.001f;
        float height = 2.0f;
        for (s32 y = 0; y <= GRID_Y; y++)
        for (s32 x = 0; x <= GRID_X; x++)
        {
            float texel_x = x / (r32)GRID_X;
            float texel_y = y / (r32)GRID_Y;
            float lens_x = (-1.0f + 2.0f * texel_x) * lens_radius / sqrt(2.0f);
            float lens_y = (-1.0f + 2.0f * texel_y) * lens_radius / sqrt(2.0f);
            float lens_z = sqrt(lens_radius*lens_radius-lens_x*lens_x-lens_y*lens_y);

            vec3 ro = m_vec3(lens_x, lens_y, lens_z);
            vec3 rd = ro / lens_radius;

            float t = (height - ro.z) / rd.z;
            vec3 p_plane = ro + t * rd;

            vec2 position, texel;
            texel.x = texel_x;
            texel.y = texel_y;
            position.x = p_plane.x;
            position.y = p_plane.y;

            Vertex v = { position, texel };
            vertices[y][x] = v;
        }

        GLuint indices[GRID_X*GRID_Y*6];
        GLuint *index = indices;
        for (s32 y = 0; y < GRID_Y; y++)
        for (s32 x = 0; x < GRID_X; x++)
        {
            GLuint v00 = y*(GRID_X+1)+x;
            GLuint v10 = y*(GRID_X+1)+x+1;
            GLuint v11 = (y+1)*(GRID_X+1)+x+1;
            GLuint v01 = (y+1)*(GRID_X+1)+x;
            *index = v00; index++;
            *index = v10; index++;
            *index = v11; index++;
            *index = v11; index++;
            *index = v01; index++;
            *index = v00; index++;
        }

        mesh_ibo = make_buffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        mesh_vbo = make_buffer(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    }
}

void tick(Input io, float t, float dt)
{
    persist int selected_frame = 28;
    persist int last_selected_frame = 0;
    persist float scale = 1.0f;

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

    #if 0
    persist s32 drag_index_x = -1;
    persist s32 drag_index_y = -1;
    if (io.mouse.left.released && drag_index_x < 0)
    {
        r32 mx = io.mouse.ndc.x/scale;
        r32 my = io.mouse.ndc.y/scale;
        drag_index_x = (s32)((0.5f + 0.5f * mx) * GRID_X+0.5f);
        drag_index_y = (s32)((0.5f + 0.5f * my) * GRID_Y+0.5f);
        if (drag_index_x < 0) drag_index_x = 0;
        if (drag_index_x > GRID_X) drag_index_x = GRID_X;
        if (drag_index_y < 0) drag_index_y = 0;
        if (drag_index_y > GRID_Y) drag_index_y = GRID_Y;
    }
    else if (io.mouse.left.released && drag_index_x >= 0)
    {
        drag_index_x = -1;
        drag_index_y = -1;
    }
    if (drag_index_x >= 0)
    {
        vertices[drag_index_y][drag_index_x].position.x = io.mouse.ndc.x/scale;
        vertices[drag_index_y][drag_index_x].position.y = io.mouse.ndc.y/scale;

        vertices[GRID_Y-drag_index_y][drag_index_x].position.x = io.mouse.ndc.x/scale;
        vertices[GRID_Y-drag_index_y][drag_index_x].position.y = -io.mouse.ndc.y/scale;

        vertices[drag_index_y][GRID_X-drag_index_x].position.x = -io.mouse.ndc.x/scale;
        vertices[drag_index_y][GRID_X-drag_index_x].position.y = io.mouse.ndc.y/scale;

        vertices[GRID_Y-drag_index_y][GRID_X-drag_index_x].position.x = -io.mouse.ndc.x/scale;
        vertices[GRID_Y-drag_index_y][GRID_X-drag_index_x].position.y = -io.mouse.ndc.y/scale;
    }
    #endif

    persist float height = 2.0f;
    // persist float lens_width_mm = 3.7f;
    // persist float lens_height_mm = 2.9f;
    // persist float lens_radius_mm = 3.7f;
    persist float lens_width_mm = 3.643f;
    persist float lens_height_mm = 2.971f;
    persist float lens_radius_mm = 3.71f;
    persist float a1 = -0.37f;
    persist float a2 = -0.42f;
    persist float a3 = 0.046f;
    persist float a4 = -0.3f;
    persist float a5 = 0.0f;
    persist float a6 = 0.54f;
    persist float a7 = -0.0f;

    a4 = a1;
    a5 = a2;
    a7 = a6;

    float lens_width = lens_width_mm * 0.001f;
    float lens_height = lens_height_mm * 0.001f;
    float lens_radius = lens_radius_mm * 0.001f;
    for (s32 y = 0; y <= GRID_Y; y++)
    for (s32 x = 0; x <= GRID_X; x++)
    {
        float texel_x = x / (r32)GRID_X;
        float texel_y = y / (r32)GRID_Y;
        float lens_x = (-1.0f + 2.0f * texel_x) * lens_width;
        float lens_y = (-1.0f + 2.0f * texel_y) * lens_height;

        float ndcx = -1.0f + 2.0f * texel_x;
        float ndcy = -1.0f + 2.0f * texel_y;

        float x2 = ndcx*ndcx;
        float x4 = x2*x2;
        float x6 = x4*x2;
        float y2 = ndcy*ndcy;
        float y4 = y2*y2;
        float y6 = y4*y2;
        float lens_z = lens_radius*(1.0f + a1*x4 + a2*x2 + a3*x2*y2 + a4*y2 + a5*y4 + a6*x6 + a7*y6);

        vec3 ro = m_vec3(lens_x, lens_y, lens_z);
        vec3 rd = m_normalize(ro);

        float t = (height - ro.z) / rd.z;
        vec3 p_plane = ro + t * rd;

        vec2 position, texel;
        texel.x = texel_x;
        texel.y = texel_y;
        position.x = p_plane.x;
        position.y = p_plane.y;

        Vertex v = { position, texel };
        vertices[y][x] = v;
    }

    glBindBuffer(GL_ARRAY_BUFFER, mesh_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    blend_mode(true);
    clearc(0.0f, 0.0f, 0.0f, 1.0f);
    begin(&pass);
    uniformi("channel0", 0);
    uniformf("aspect", WINDOW_WIDTH / (r32)WINDOW_HEIGHT);
    uniformf("scale", scale);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_ibo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh_vbo);
    attribfv("position", 2, 4, 0);
    attribfv("texel", 2, 4, 2);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    uniformf("wireframe", 0.0f);
    glDrawElements(GL_TRIANGLES, GRID_X*GRID_Y*6, GL_UNSIGNED_INT, 0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    uniformf("wireframe", 1.0f);
    glDrawElements(GL_TRIANGLES, GRID_X*GRID_Y*6, GL_UNSIGNED_INT, 0);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    ImGui::NewFrame();
    ImGui::SliderFloat("lw", &lens_width_mm, 2.0f, 5.0f);
    ImGui::SliderFloat("lh", &lens_height_mm, 2.0f, 5.0f);
    ImGui::SliderFloat("radius", &lens_radius_mm, 2.0f, 5.0f);
    ImGui::SliderFloat("scale", &scale, 0.02f, 1.0f);
    ImGui::SliderFloat("a1", &a1, -1.0f, 1.0f);
    ImGui::SliderFloat("a2", &a2, -1.0f, 1.0f);
    ImGui::SliderFloat("a3", &a3, -1.0f, 1.0f);
    ImGui::SliderFloat("a4", &a4, -1.0f, 1.0f);
    ImGui::SliderFloat("a5", &a5, -1.0f, 1.0f);
    ImGui::SliderFloat("a6", &a6, -1.0f, 1.0f);
    ImGui::SliderFloat("a7", &a7, -1.0f, 1.0f);
    ImGui::SliderInt("frame", &selected_frame, 1, 105);
    ImGui::Render();
}

#include "sumo.cpp"
