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
uniform mat4 view;
uniform sampler2D channel0;
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

void main()
{
    vec2 xy = quadcoord * vec2(aspect, 1.0);

    // Hacky fisheye model
    // float z = sqrt(1.4 - dot(xy, xy));
    // vec3 rd = normalize(vec3(xy, z));

    // Equidistant
    // float r = length(xy);
    // float theta = r * 1.5;
    // float phi = atan(xy.y, xy.x);
    // vec3 rd = normalize(vec3(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta)));

    // Stereographic
    // float r = length(xy);
    // float theta = 2.0 * atan(r / 2.0);
    // float phi = atan(xy.y, xy.x);
    // vec3 rd = normalize(vec3(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta)));

    // Equisolid
    // float r = length(xy);
    // float theta = 2.0 * asin(r / 2.0);
    // float phi = atan(xy.y, xy.x);
    // vec3 rd = normalize(vec3(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta)));

    // Orthogonal
    float r = length(xy);
    if (r > 1.0)
    {
        f_color = vec4(0.0);
        return;
    }
    float theta = asin(r);
    float phi = atan(xy.y, xy.x);
    vec3 rd = normalize(vec3(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta)));

    rd = normalize((view * vec4(rd, 0.0)).xyz);
    vec3 ro = view[3].xyz;

    vec2 plane;
    if (trace(ro, rd, plane))
    {
        vec2 floor_texel = vec2(0.5) + 0.5 * plane / vec2(PLANE_SIZE);
        f_color.rgb = texture(channel0, floor_texel).rgb;
        f_color.rgb *= 1.0 - 0.2*length(quadcoord);
    }
    else
    {
        f_color.rgb = vec3(0.84, 0.82, 0.75) * (1.0 + 0.1*abs(rd.y));
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
    uniformi("channel0", 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    ImGui::NewFrame();
    ImGui::Begin("Camera");
    ImGui::SliderFloat("x", &camera_x, 0.1f, 3.0f);
    ImGui::SliderFloat("y", &camera_y, 0.1f, 3.0f);
    ImGui::SliderFloat("z", &camera_z, 0.1f, 3.0f);
    ImGui::SliderAngle("phi", &camera_phi, -180.0f, 180.0f);
    ImGui::SliderAngle("theta", &camera_theta, -180.0f, 180.0f);
    ImGui::SliderAngle("psi", &camera_psi, -180.0f, 180.0f);
    ImGui::End();
    ImGui::Render();
}

#include "sumo.cpp"
