#define USE_NEW_MATH
#include "sumo.h"
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define GLSL(src) "#version 150\n" #src

char *vs = GLSL(
in vec3 position;
in vec3 normal;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
out vec3 v_normal;
void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
    v_normal = (model * vec4(normal, 0.0)).xyz;
}
);

char *fs = GLSL(
in vec3 v_normal;
uniform vec3 albedo;
uniform vec3 ambient;
out vec4 f_color;
void main()
{
    vec3 color = vec3(0.0);
    vec3 n = normalize(v_normal);
    vec3 l = normalize(vec3(1.0, 1.2, 0.8));
    color += max(dot(n, l), 0.0) * albedo;
    color += ambient;
    f_color.rgb = color;
    f_color.a = 1.0;
}
);

RenderPass pass;
Mesh cube;

void init()
{
    pass = make_render_pass(vs, fs);
    cube = make_cube();
}

void draw_cube(vec3 albedo, mat4 model)
{
    uniformf("model", model);
    uniformf("albedo", albedo);
    uniformf("ambient", m_vec3(0.05, 0.07, 0.1));
    glDrawElements(GL_TRIANGLES, cube.index_count, cube.index_type, 0);

    uniformf("model", mat_scale(1.0f, 0.001f, 1.0f)*model);
    uniformf("albedo", m_vec3(0.0f, 0.0f, 0.0f));
    uniformf("ambient", 0.8f*m_vec3(0.8f, 0.7f, 0.6f));
    glDrawElements(GL_TRIANGLES, cube.index_count, cube.index_type, 0);
}

void tick(Input io, float t, float dt)
{
    float aspect = WINDOW_WIDTH / (float)WINDOW_HEIGHT;
    float zoom = 2.5f;
    mat4 projection = mat_ortho_depth(-aspect*zoom, aspect*zoom, -zoom, zoom, 0.1f, 10.0f);
    mat4 view = mat_translate(0.0f, 0.0f, -8.0f) * mat_rotate_x(0.7f) * mat_rotate_y(0.3f);

    begin(&pass);
    depth_test(true, GL_LEQUAL);
    depth_write(true);
    clear(0.8f, 0.7f, 0.6f, 1.0f, 1.0f);
    glBindBuffer(GL_ARRAY_BUFFER, cube.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.ibo);
    attribfv("position", 3, 6, 0);
    attribfv("normal", 3, 6, 3);
    uniformf("projection", projection);
    uniformf("view", view);

    draw_cube(m_vec3(1.0f, 0.3f, 0.1f), mat_translate(1.0f, 0.0f, 0.0f)*mat_scale(1.0f, 0.01f, 0.01f));
    draw_cube(m_vec3(0.1f, 1.0f, 0.3f), mat_translate(0.0f, 1.0f, 0.0f)*mat_scale(0.01f, 1.0f, 0.01f));
    draw_cube(m_vec3(0.3f, 0.2f, 1.0f), mat_translate(0.0f, 0.0f, 1.0f)*mat_scale(0.01f, 0.01f, 1.0f));

    struct Frame
    {
        float theta;
        float d;
        float a;
        float alpha;
    };

    #if 0
    persist float d1 = 1.0f;
    persist float d2 = 1.0f;

    Frame frames[] = {
        { 0.0f, d1, 0.0f, PI / 2.0f },
        { 0.0f, d2, 0.0f, 0.0f}
    };

    mat4 transform = mat_rotate_x(-PI / 2.0f);
    for (int i = 0; i < array_count(frames); i++)
    {
        Frame frame = frames[i];
        float theta = frame.theta;
        float d = frame.d;
        float a = frame.a;
        float alpha = frame.alpha;
        mat4 f = mat_rotate_z(theta)*
                 mat_translate(0.0f, 0.0f, d)*
                 mat_translate(a, 0.0f, 0.0f)*
                 mat_rotate_x(alpha);
        transform = transform * f;
        draw_cube(m_vec3(0.5f, 0.7f, 1.0f), transform*mat_scale(0.2f));
    }

    ImGui::NewFrame();
    ImGui::SliderFloat("d1", &d1, 1.0f, 2.0f);
    ImGui::SliderFloat("d2", &d2, 1.0f, 2.0f);
    ImGui::Render();
    #endif

    #if 0
    persist float d2 = 1.0f;
    persist float theta1 = 0.0f;
    persist float theta3 = 0.0f;

    Frame frames[] = {
        { theta1 + PI/2.0f, 0.0f, 0.0f, PI/2.0f },
        { 0.0f, d2, 0.0f, -PI / 2.0f },
        { -PI/2.0f+theta3, 0.0f, 1.0f, 0.0f }
    };

    mat4 transform = m_id4();
    for (int i = 0; i < array_count(frames); i++)
    {
        Frame frame = frames[i];
        float theta = frame.theta;
        float d = frame.d;
        float a = frame.a;
        float alpha = frame.alpha;
        mat4 f = mat_rotate_z(theta)*
                 mat_translate(0.0f, 0.0f, d)*
                 mat_translate(a, 0.0f, 0.0f)*
                 mat_rotate_x(alpha);
        transform = transform * f;
        draw_cube(m_vec3(0.5f, 0.7f, 1.0f), transform*mat_scale(0.2f));
    }

    ImGui::NewFrame();
    ImGui::SliderFloat("d2", &d2, 1.0f, 2.0f);
    ImGui::SliderAngle("theta1", &theta1);
    ImGui::SliderAngle("theta3", &theta3);
    ImGui::Render();
    #endif

    #if 0
    persist float d1 = 1.0f;
    persist float d2 = 1.0f;
    persist float d3 = 1.0f;

    Frame frames[] = {
        { 0.0f, d1, 0.0f, PI/2.0f },
        { PI/2.0f, d2, 0.0f, PI/2.0f },
        { 0.0f, d3, 0.0f, 0.0f },
    };

    mat3 R = {};
    R.a1 = m_vec3(0.0f, -1.0f, 0.0f);
    R.a2 = m_vec3(1.0f, 0.0f, 0.0f);
    R.a3 = m_vec3(0.0f, 0.0f, 1.0f);
    mat4 transform = m_se3(R, m_vec3(0.0f, 0.0f, 0.0f));
    for (int i = 0; i < array_count(frames); i++)
    {
        Frame frame = frames[i];
        float theta = frame.theta;
        float d = frame.d;
        float a = frame.a;
        float alpha = frame.alpha;
        mat4 f = mat_rotate_z(theta)*
                 mat_translate(0.0f, 0.0f, d)*
                 mat_translate(a, 0.0f, 0.0f)*
                 mat_rotate_x(alpha);
        transform = transform * f;
        draw_cube(m_vec3(0.5f, 0.7f, 1.0f), transform*mat_scale(0.2f));
    }

    ImGui::NewFrame();
    ImGui::SliderFloat("d1", &d1, 1.0f, 2.0f);
    ImGui::SliderFloat("d2", &d2, 1.0f, 2.0f);
    ImGui::SliderFloat("d3", &d3, 1.0f, 2.0f);
    ImGui::Render();
    #endif

    // TODO: Wrong
    #if 1
    persist float t1 = 0.0f;
    persist float t2 = 0.0f;
    persist float t3 = 0.0f;
    persist float t4 = 0.0f;
    persist float t5 = 0.0f;
    persist float t6 = 0.0f;

    Frame frames[] = {
        { t1, 1.0f, 0.0f, PI/2.0f },
        // { t2, 0.0f, 1.0f, 0.0f },
        // { t3-PI/2.0f, 0.0f, 0.0f, PI/2.0f },
    };

    mat3 R = {};
    R.a1 = m_vec3(-1.0f, 0.0f, 0.0f);
    R.a2 = m_vec3(0.0f, 0.0f, 1.0f);
    R.a3 = m_vec3(0.0f, 1.0f, 0.0f);
    mat4 transform = m_se3(R, m_vec3(0.0f, 0.0f, 0.0f));
    for (int i = 0; i < array_count(frames); i++)
    {
        Frame frame = frames[i];
        float theta = frame.theta;
        float d = frame.d;
        float a = frame.a;
        float alpha = frame.alpha;
        mat4 f = mat_rotate_z(theta)*
                 mat_translate(0.0f, 0.0f, d)*
                 mat_translate(a, 0.0f, 0.0f)*
                 mat_rotate_x(alpha);
        transform = transform * f;
        draw_cube(m_vec3(0.5f, 0.7f, 1.0f), transform*mat_scale(0.2f));

        draw_cube(m_vec3(1.0f, 0.3f, 0.1f), transform*mat_translate(0.25f, 0.0f, 0.0f)*mat_scale(0.25f, 0.01f, 0.01f));
        draw_cube(m_vec3(0.1f, 1.0f, 0.3f), transform*mat_translate(0.0f, 0.25f, 0.0f)*mat_scale(0.01f, 0.25f, 0.01f));
        draw_cube(m_vec3(0.3f, 0.2f, 1.0f), transform*mat_translate(0.0f, 0.0f, 0.25f)*mat_scale(0.01f, 0.01f, 0.25f));
    }

    ImGui::NewFrame();
    ImGui::SliderAngle("t1", &t1);
    ImGui::SliderAngle("t2", &t2);
    ImGui::SliderAngle("t3", &t3);
    ImGui::SliderAngle("t4", &t4);
    ImGui::SliderAngle("t5", &t5);
    ImGui::SliderAngle("t6", &t6);
    ImGui::Render();
    #endif
}

#include "sumo.cpp"
