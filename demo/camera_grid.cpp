#define USE_NEW_MATH
#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define GLSL150(src) "#version 150\n" #src

char *VS = GLSL150(
in vec2 position;
out vec2 quadcoord;
void main()
{
    quadcoord = position;
    gl_Position = vec4(position, 0.0, 1.0);
}
);

char *FS = GLSL150(
in vec2 quadcoord;
uniform float aspect;
uniform float focal_distance;
uniform float k1;
uniform float k2;
uniform mat4 view;
uniform sampler2D channel0;
uniform float time;
uniform vec2 pixel_size;
out vec4 f_color;

const float PLANE_SIZE = 5.0;

bool trace(vec3 ro, vec3 rd, out vec2 p)
{
    float t = -ro.y / rd.y;
    if (t >= 0.0)
    {
        p = (ro + rd * t).xz;
        if (abs(p.x) <= PLANE_SIZE &&
            abs(p.y) <= PLANE_SIZE)
            return true;
        else
            return false;
    }
    else
    {
        return false;
    }
}

vec2 seed = (quadcoord) * (time + 1.0);
vec2 noise2f() {
    seed += vec2(-1, 1);
    // implementation based on: lumina.sourceforge.net/Tutorials/Noise.html
    return vec2(fract(sin(dot(seed.xy, vec2(12.9898, 78.233))) * 43758.5453),
        fract(cos(dot(seed.xy, vec2(4.898, 7.23))) * 23421.631));
}

void main()
{
    vec2 sample = quadcoord;
    float r = length(sample);
    sample += (-1.0 + 2.0 * noise2f()) * (1.0 + 4.0*r*r) * pixel_size;
    sample = (1.0 + k1*r*r + k2*r*r*r*r) * sample;
    vec3 film = vec3(sample.x * aspect, sample.y, focal_distance);
    vec3 origin = (view[3]).xyz;
    vec3 dir = normalize((view * vec4(film, 1.0)).xyz - origin);

    vec2 plane;
    if (trace(origin, dir, plane))
    {
        vec2 floor_texel = vec2(0.5) + 0.5 * plane / vec2(PLANE_SIZE);
        f_color.rgb = texture(channel0, floor_texel).rgb;
        f_color.rgb *= 1.0 - 0.2*length(quadcoord);
    }
    else
    {
        f_color.rgb = vec3(0.84, 0.82, 0.75) * (1.0 + 0.1*abs(dir.y));
    }
    f_color.a = 1.0;
}
);

RenderPass pass;
GLuint quad;
GLuint tex;

void init()
{
    pass = make_render_pass(VS, FS);
    quad = make_quad();
    tex = so_load_tex2d("C:/Resources/textures/kitchen-tiles.jpg",
                        0, 0, GL_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT);
}

void tick(Input io, float t, float dt)
{
    float aspect = (float)WINDOW_WIDTH / WINDOW_HEIGHT;
    persist float focal_distance = 1.05f;
    persist float camera_height = 1.0f;
    persist float k1 = 0.0f;
    persist float k2 = 0.0f;

    // mat4 view = mat_rotate_y(0.0f) *
    //             mat_rotate_x(-PI / 2.0f + 0.2f*sin(0.5f*t)) *
    //             mat_translate(0.0f, sin(t)*0.2f, camera_height);

    persist float camera_x = 0.0f;
    persist float camera_y = 1.3f;
    persist float camera_z = 0.0f;
    persist float camera_phi = 0.0f; // roll
    persist float camera_theta = PI / 2.0f; // pitch
    persist float camera_psi = 0.0f; // yaw

    // Some animation
    #if 0
    focal_distance = 0.5f;
    k1 = 0.1f;
    k2 = 0.05f;
    camera_x = 0.6f*sin(t) + 0.03f*sin(5.4f*t);
    camera_z = 0.6f*cos(t) + 0.03f*cos(5.4f*t);
    camera_y = 1.3f;
    camera_theta = PI / 2.0f;
    #endif

    mat4 view = mat_translate(camera_x, camera_y, camera_z) *
                mat_rotate_z(camera_phi) *
                mat_rotate_x(camera_theta) *
                mat_rotate_y(camera_psi);

    clearc(1.0f, 1.0f, 1.0f, 1.0f);
    begin(&pass);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glBindBuffer(GL_ARRAY_BUFFER, quad);
    attribfv("position", 2, 2, 0);
    uniformf("aspect", aspect);
    uniformf("view", view);
    uniformf("focal_distance", focal_distance);
    uniformf("k1", k1);
    uniformf("k2", k2);
    uniformf("time", t);
    uniformf("pixel_size", m_vec2(1.0f / WINDOW_WIDTH, 1.0f / WINDOW_HEIGHT));
    uniformi("channel0", 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    ImGui::NewFrame();
    ImGui::Begin("Camera");
    ImGui::SliderFloat("focal_distance", &focal_distance, 0.1f, 3.0f);
    ImGui::SliderFloat("x", &camera_x, 0.1f, 3.0f);
    ImGui::SliderFloat("y", &camera_y, 0.1f, 3.0f);
    ImGui::SliderFloat("z", &camera_z, 0.1f, 3.0f);
    ImGui::SliderAngle("phi", &camera_phi, -180.0f, 180.0f);
    ImGui::SliderAngle("theta", &camera_theta, -180.0f, 180.0f);
    ImGui::SliderAngle("psi", &camera_psi, -180.0f, 180.0f);
    if (ImGui::CollapsingHeader("Brown-Conrady coefficients"))
    {
        ImGui::SliderFloat("k1", &k1, -1.0f, 1.0f);
        ImGui::SliderFloat("k2", &k2, -1.0f, 1.0f);
    }
    ImGui::End();
    ImGui::Render();
}

#include "sumo.cpp"
