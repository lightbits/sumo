#define USE_NEW_MATH
#include "sumo.h"
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define GLSL(src) "#version 150\n" #src

char *vs = GLSL(
in  vec2 position;
out vec2 VS_POSITION;
void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
    VS_POSITION = position;
}
);

char *fs = GLSL(
in      vec2      VS_POSITION;
uniform vec3      U_FORWARD;
uniform vec3      U_RIGHT;
uniform vec3      U_UP;
uniform vec3      U_ORIGIN;
uniform float     U_ASPECT;
uniform sampler2D TEX_DIFFUSE_WARP;
out     vec4      FS_COLOR;
float MAP(vec3 p)
{
    // float d1 = length(p) - 0.5;
    // float d2 = length(p - vec3(0.8, 0.1, 0.0)) - 0.2;
    // float d3 = length(p - vec3(-0.5, 0.1, 0.0)) - 0.4;
    // float d4 = length(p - vec3(0.1, 0.6, 0.0)) - 0.8;
    // return min(min(min(d1, d2), d3), d4);
    return length(p) - 1.0;
}

vec3 NORMAL(vec3 p)
{
    vec2 e = vec2(0.001, 0.0);
    return normalize(vec3(
                     MAP(p + e.xyy) - MAP(p - e.xyy),
                     MAP(p + e.yxy) - MAP(p - e.yxy),
                     MAP(p + e.yyx) - MAP(p - e.yyx)));
}

vec3 SHADE_AMBIENT(vec3 n)
{
    return vec3(0.0);
}

vec3 SHADE_LIGHT(vec3 v, vec3 n, vec3 ci, vec3 li)
{
    float nl = 0.5+0.5*dot(li, n);
    float nv = dot(n, v);
    vec3 w = texture(TEX_DIFFUSE_WARP, vec2(nl, nv)).rgb;
    w = pow(w, vec3(1.0 / 0.4545));
    return ci*2.0*w;
}

vec3 SHADE(vec3 rd, vec3 p)
{
    // LIGHT ARRAY
    vec3 c0 = vec3(1.0, 0.95, 0.92);
    vec3 l0 = normalize(vec3(0.5, 1.0, 0.5));
    // ...

    vec3 normal = NORMAL(p);
    vec3 toview = -rd;
    vec3 albedo = vec3(1.0, 0.9, 0.9);
    vec3 color = vec3(0.0, 0.0, 0.0);
    color += SHADE_AMBIENT(normal);
    color += SHADE_LIGHT(toview, normal, c0, l0);
    color *= albedo;
    return color;
}

vec3 GAMMA(vec3 v)
{
    return pow(v, vec3(0.4545));
}

vec3 INV_GAMMA(vec3 v)
{
    return pow(v, vec3(1.0 / 0.4545));
}

vec3 TRACE(vec3 ro, vec3 rd)
{
    float t = 0.0;
    for (int step = 0; step < 64; step++)
    {
        float d = MAP(ro + rd * t);
        if (d < 0.001)
        {
            return SHADE(rd, ro + rd * t);
        }
        t += d;
    }
    return INV_GAMMA(vec3(0.21, 0.38, 0.51));
}

void main()
{
    vec3 ro = U_ORIGIN;
    vec3 rd = normalize(U_FORWARD*1.0 + U_RIGHT*VS_POSITION.x*U_ASPECT + U_UP*VS_POSITION.y);
    FS_COLOR.rgb = TRACE(ro, rd);
    FS_COLOR.rgb = GAMMA(FS_COLOR.rgb);
    FS_COLOR.a = 1.0;
}
);

RenderPass pass;
GLuint tex_diffuse_warp;

void init()
{
    pass = make_render_pass(vs, fs);
    tex_diffuse_warp = so_load_tex2d("demo/tf2/warp1.png");
}

void tick(Input io, float t, float dt)
{
    // Camera
    mat3 frame;
    vec3 p;
    {
        persist float theta = 0.0f;
        persist float phi = 0.0f;
        persist float Dtheta = 0.0f;
        persist float Dphi = 0.0f;
        float DDtheta = 0.0f;
        float DDphi = 0.0f;
        if (io.mouse.left.down && !ImGui::GetIO().WantCaptureMouse)
        {
            DDtheta -= dt * 10.0f * io.mouse.rel.x;
            DDphi -= dt * 10.0f * io.mouse.rel.y;
        }
        DDtheta -= 1.0f * Dtheta;
        DDphi -= 1.0f * Dphi;
        Dtheta += DDtheta * dt;
        Dphi += DDphi * dt;
        theta += Dtheta * dt;
        phi += Dphi * dt;
        frame = m_mat3(mat_rotate_y(theta)*mat_rotate_x(phi));
        p = 2.0f*(frame * m_vec3(0.0f, 0.0f, 1.0f));
    }

    clearc(0.35f, 0.55f, 1.0f, 1.0f);
    begin(&pass);
    {
        glBindTexture(GL_TEXTURE_2D, tex_diffuse_warp);
        uniformi("TEX_DIFFUSE_WARP", 0);
        uniformf("U_FORWARD", -frame.a3);
        uniformf("U_RIGHT", frame.a1);
        uniformf("U_UP", frame.a2);
        uniformf("U_ORIGIN", p);
        uniformf("U_ASPECT", WINDOW_WIDTH / (float)WINDOW_HEIGHT);
        so_draw_fullscreen_quad();
    }

    ImGui::NewFrame();
    {
    }
    ImGui::Render();
}

#include "sumo.cpp"
