#define USE_NEW_MATH
#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS
#define GLSL(src) "#version 150 \n" #src

char *diffuse_vs = GLSL(
in vec3 position;
in vec3 normal;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
out vec3 v_position;
out vec3 v_normal;
void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
    v_position = (view * model * vec4(position, 1.0)).xyz;
    v_normal = (view * model * vec4(normal, 0.0)).xyz;
}
);

char *diffuse_fs = GLSL(
in vec3 v_position; // view-space
in vec3 v_normal;   // view-space
uniform mat4 view;
uniform float emissive;
uniform float roughness;
uniform float shininess;
uniform float albedo;
out vec4 f_color;

float OrenNayar(vec3 lightDirection,
                vec3 viewDirection,
                vec3 surfaceNormal,
                float roughness,
                float albedo)
{
    float NdotL = max(dot(lightDirection, surfaceNormal), 0.0);
    float LdotE = max(dot(lightDirection, viewDirection), 0.0);
    float NdotE = max(dot(viewDirection, surfaceNormal), 0.0f);

    //float s = roughness*roughness;
    // Let's instead just assume roughness = sigma^2
    float s = roughness;
    float A = 1.0 - 0.5 * s / (s + 0.33);
    float B = 0.45 * s / (s + 0.09);
    float k1 = max(0.0, LdotE-NdotE*NdotL);
    float k2 = min(1.0, NdotL / NdotE);

    return albedo * (NdotL * A + B * k1 * k2) / 3.1415926;
}

float Specular(vec3 lightDirection,
               vec3 viewDirection,
               vec3 surfaceNormal,
               float shininess)
{
    vec3 H = normalize(lightDirection + viewDirection);
    float w = (1.0 - max(dot(H, surfaceNormal), 0.0)) / shininess;
    return exp(-w*w);
}

void main()
{
    vec3 L1 = normalize((view * vec4(0, 1, 0, 0)).xyz);
    vec3 L2 = normalize(vec3(0.0, 0.5, 1.0));
    vec3 N = normalize(v_normal);
    vec3 E = -normalize(v_position);

    vec3 c1 = 2.5*vec3(1.0, 0.98, 0.8);
    vec3 c2 = 0.2*vec3(1.3, 1.2, 1.0);

    vec3 color = vec3(0.0);
    color += c1 * OrenNayar(L1, E, N, roughness, albedo);
    color += c2 * OrenNayar(L2, E, N, roughness, albedo);
    color += c1 * vec3(1.0) * Specular(L1, E, N, shininess);
    color += c2 * vec3(1.0) * Specular(L2, E, N, shininess);

    f_color.rgb = color;
    f_color.rgb = pow(f_color.rgb, vec3(0.4545)); // gamma correct
    f_color.rgb = mix(f_color.rgb, vec3(1.0), emissive);
    f_color.a = emissive;
}
);

char *post_vs = GLSL(
in vec2 position;
out vec2 v_position;
void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
    v_position = position;
}
);

char *post_fs = GLSL(
in vec2 v_position;
uniform sampler2D channel0;
uniform float time;
out vec4 f_color;
const int SAMPLES = 24;
float random(vec2 co)
{
    co += vec2(0.53*time, 0.701*time);
    return fract(sin(dot(co.xy,vec2(12.9898,78.233))) * 43758.5453);
}
void main()
{
    vec2 uv = vec2(0.5) + 0.5 * v_position;

    vec2 glowCenter = vec2(0.0, 0.0);
    vec2 p = v_position;
    vec2 dp = 0.5 * (v_position - glowCenter) / float(SAMPLES);
    float glow = 0.0;
    float weight = 1.0;
    for (int i = 0; i < SAMPLES; i++)
    {
        float data = texture(channel0, vec2(0.5) + 0.5 * p).a;
        glow += data * weight * mix(0.95, 1.0, random(uv));
        weight *= 0.92;
        p -= dp * mix(0.8, 1.0, random(uv));
    }

    vec4 pass = texture(channel0, uv);
    f_color.rgb = pass.rgb + vec3(pass.a);
    f_color.rgb += 0.1 * glow * vec3(1.3, 1.2, 1.0);

    f_color.rgb += 0.1*vec3(1.0-0.3*uv.y, 0.0, 0.5+0.5*uv.x);
    f_color.a = 1.0;
}
);

MeshAsset mesh;
struct RenderPasses
{
    RenderPass diffuse;
    RenderPass post;
} pass;

so_Framebuffer fbo;

void init()
{
    mesh = load_mesh("assets/models/mitsuba/mitsuba.sumo_asset",
                     MESH_NORMALIZE);
    pass.diffuse = make_render_pass(diffuse_vs, diffuse_fs);
    pass.post = make_render_pass(post_vs, post_fs);
    so_make_fbo_rgbad(&fbo, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA8);
}

void tick(Input io, float t, float dt)
{
    mat4 projection = mat_perspective(PI / 3.5f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.02f, 10.0f);
    mat4 view = mat_translate(0.0f, -0.1f, -1.5f) * mat_rotate_x(0.3f) * mat_rotate_y(t);

    static float shininess = 0.03f;
    static float roughness = 1.0f;
    static float albedo = 1.0f;

    begin(&pass.diffuse);
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo.fbo);
        glViewport(0, 0, fbo.width, fbo.height);
        depth_test(true);
        depth_write(true);
        clear(0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

        glBindBuffer(GL_ARRAY_BUFFER, mesh.positions);
        attribfv("position", 3, 3, 0);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.normals);
        attribfv("normal", 3, 3, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indices);

        uniformf("projection", projection);
        uniformf("view", view);

        uniformf("model", mat_scale(0.5f));
        uniformf("emissive", 0.0f);
        uniformf("albedo", albedo);
        uniformf("shininess", shininess);
        uniformf("roughness", roughness);
        glDrawElements(GL_TRIANGLES, mesh.num_indices, GL_UNSIGNED_INT, 0);

        uniformf("model", mat_translate(0.0f, 0.3f, -0.07f)*
                          mat_scale(0.33f)*
                          mat_rotate_x(PI/2.0f));
        uniformf("emissive", 1.0f);
        glDrawElements(GL_TRIANGLES, mesh.num_indices, GL_UNSIGNED_INT, 0);
    }

    begin(&pass.post);
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        depth_test(false);
        depth_write(false);
        clearc(0.0f, 0.0f, 0.0f, 1.0f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fbo.color[0]);
        uniformf("time", t);
        uniformi("channel0", 0);
        so_draw_fullscreen_quad();
    }

    ImGui::NewFrame();
    ImGui::Begin("Shader");
    ImGui::SliderFloat("shininess", &shininess, 0.01f, 0.2f);
    ImGui::SliderFloat("roughness", &roughness, 0.0f, 1.0f);
    ImGui::SliderFloat("albedo", &albedo, 0.0f, 1.0f);
    ImGui::End();
    ImGui::Render();
}

#include "sumo.cpp"
