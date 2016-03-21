#define USE_NEW_MATH
#include "sumo.h"
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define GLSL(src) "#version 150\n" #src

char *vs = GLSL(
in vec2 position;
in vec2 texel;
uniform float scale;
uniform float aspect;
out vec2 v_texel;
void main()
{
    v_texel = texel;
    gl_Position = vec4(vec2(1.0, aspect) * position, 0.0, 1.0);
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
    f_color = mix(f_color, vec4(1.0, 1.0, 1.0, 0.01), wireframe);
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
        for (s32 y = 0; y <= GRID_Y; y++)
        for (s32 x = 0; x <= GRID_X; x++)
        {
            Vertex v = {0};
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
    static float aspect_ratio = 1.0f;
    static int width = 0;
    static int height = 0;

    glBindTexture(GL_TEXTURE_2D, tex);
    if (last_selected_frame != selected_frame)
    {
        last_selected_frame = selected_frame;
        char path[256];
        sprintf(path, "C:/Downloads/temp/video4/video%04d.png", selected_frame);
        // char *path = "C:/Downloads/temp/video4/test-beach.jpg";
        s32 channels;
        u08 *data = stbi_load(path, &width, &height, &channels, 3);
        ASSERT(data);

        aspect_ratio = width / (float)height;

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }

    r32 cam_fisheye_radius = 1470.0f/2.0f;
    r32 cam_fisheye_fov = PI;
    static r32 cam_tcap = PI/2.2f;
    static r32 cam_rectilinear_fov_x = PI/2.0f;
    static r32 scale = 1000.0f;
    r32 cam_rectilinear_f = (width/2.0f)/tan(cam_rectilinear_fov_x/2.0f);

    for (s32 yi = 0; yi <= GRID_Y; yi++)
    for (s32 xi = 0; xi <= GRID_X; xi++)
    {
        r32 xd = (-1.0f + 2.0f * xi / GRID_X) * width / 2.0f;
        r32 yd = (+1.0f - 2.0f * yi / GRID_Y) * height / 2.0f;
        r32 rd = sqrt(xd*xd+yd*yd);
        r32 theta = (cam_fisheye_fov/2.0f)*rd/cam_fisheye_radius;
        if (theta > cam_tcap) theta = cam_tcap;
        r32 ru = tan(theta);

        // xu and yu are now in rectilinear image coordinates
        // [-w/2 w/2]x[-h/2 h/2]
        r32 xu, yu;
        if (rd > 1.0f)
        {
            xu = cam_rectilinear_f*(xd/rd)*ru;
            yu = cam_rectilinear_f*(yd/rd)*ru;
        }
        else // Handle limit case in center
        {
            xu = cam_rectilinear_f*xd*ru;
            yu = cam_rectilinear_f*yd*ru;
        }

        Vertex v;
        v.position.x = xu/scale;
        v.position.y = yu/scale;
        v.texel.x = (r32)xi / GRID_X;
        v.texel.y = (r32)yi / GRID_Y;

        vertices[yi][xi] = v;
    }

    glBindBuffer(GL_ARRAY_BUFFER, mesh_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    blend_mode(true);
    clearc(0.0f, 0.0f, 0.0f, 1.0f);
    begin(&pass);
    uniformi("channel0", 0);
    uniformf("aspect", WINDOW_WIDTH / (r32)WINDOW_HEIGHT);
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
    ImGui::SliderFloat("scale", &scale, 100.0f, 2000.0f);
    ImGui::SliderAngle("rectilinear fov", &cam_rectilinear_fov_x);
    ImGui::SliderAngle("tcap", &cam_tcap);
    ImGui::SliderInt("frame", &selected_frame, 1, 105);
    ImGui::Render();
}

#include "sumo.cpp"
