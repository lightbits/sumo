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
uniform float lens_width;
uniform float lens_height;
uniform float focal_distance;
uniform float k1;
uniform float k2;
uniform vec3 hover_point;
uniform mat4 c_to_w;
uniform sampler2D channel0;
out vec4 f_color;

const float PLANE_SIZE = 2.5;

bool trace(vec3 ro, vec3 rd, out vec2 p)
{
    float t = -ro.z / rd.z;
    if (t >= 0.0)
    {
        p = (ro + rd * t).xy;
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

void main()
{
    vec2 sample = quadcoord;
    float r = length(sample);
    sample = (1.0 + k1*r*r + k2*r*r*r*r) * sample;

    vec3 film = vec3(sample.x * lens_width/2.0,
                     sample.y * lens_height/2.0, -focal_distance);

    vec3 origin = (c_to_w[3]).xyz;
    vec3 dir = normalize((c_to_w * vec4(film, 1.0)).xyz - origin);

    vec2 plane;
    if (trace(origin, dir, plane))
    {
        vec2 floor_texel = vec2(0.5) + 0.5 * plane / vec2(PLANE_SIZE);
        f_color.rgb = texture(channel0, floor_texel).rgb;
        f_color.rgb *= 1.0 - 0.2*length(quadcoord);

        if (length(plane.xy-hover_point.xy) < 0.25)
        {
            f_color.r += 0.5;
        }
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

vec3 trace_floor(mat3 c_to_w, vec3 ro, vec3 pc)
{
    vec3 rd = c_to_w * m_normalize(pc);
    float t = -ro.z / rd.z;
    return ro + rd * t;
}

void tick(Input io, float t, float dt)
{
    float aspect = (float)WINDOW_WIDTH / WINDOW_HEIGHT;
    persist float focal_distance_mm = 3.67f;
    persist float k1 = 0.0f; // 0.1f;
    persist float k2 = 0.0f; // 0.05f;
    float lens_width = 4.8f * 0.001f;
    float lens_height = 3.6f * 0.001f;
    float focal_distance = focal_distance_mm * 0.001f;

    persist vec3  camera_p = m_vec3(0.0f, 0.0f, 1.3f);
    persist float camera_phi = 0.0f;
    persist float camera_theta = 0.0f;
    persist float camera_psi = 0.0f;

    mat3 camera_R = m_mat3(mat_rotate_x(camera_theta)*
                           mat_rotate_y(camera_phi)*
                           mat_rotate_z(camera_psi));

    mat4 c_to_w = m_se3(camera_R, camera_p);

    vec3 hover_point;
    {
        float pc_x = io.mouse.ndc.x * lens_width/2.0f;
        float pc_y = io.mouse.ndc.y * lens_height/2.0f;
        float pc_z = -focal_distance;
        vec3 pc = m_vec3(pc_x, pc_y, pc_z);
        vec3 pw = trace_floor(camera_R, camera_p, pc);
        hover_point = pw;
    }

    clearc(1.0f, 1.0f, 1.0f, 1.0f);
    begin(&pass);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glBindBuffer(GL_ARRAY_BUFFER, quad);
    attribfv("position", 2, 2, 0);
    uniformf("c_to_w", c_to_w);
    uniformf("lens_width", lens_width);
    uniformf("lens_height", lens_height);
    uniformf("hover_point", hover_point);
    uniformf("focal_distance", focal_distance);
    uniformf("k1", k1);
    uniformf("k2", k2);
    uniformi("channel0", 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    ImGui::NewFrame();
    ImGui::Begin("Camera");
    ImGui::PushItemWidth(0.4f * ImGui::GetWindowWidth());
    ImGui::SliderFloat("focal_distance [mm]", &focal_distance_mm, 1.0f, 10.0f);
    ImGui::PopItemWidth();
    ImGui::Text("%.2f %.2f %.2f", hover_point.x, hover_point.y, hover_point.z);
    ImGui::SliderFloat("x", &camera_p.x, 0.1f, 3.0f);
    ImGui::SliderFloat("y", &camera_p.y, 0.1f, 3.0f);
    ImGui::SliderFloat("z", &camera_p.z, 0.1f, 3.0f);
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
