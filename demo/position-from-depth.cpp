#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS
#define GLSL150(src) "#version 150\n" #src

char *GEOMETRY_VS = GLSL150(
    in vec3 position;
    in vec3 normal;
    out vec3 view_normal;
    out float z_view;
    uniform mat4 projection;
    uniform mat4 view;
    uniform mat4 model;
    void main()
    {
        vec4 view_position = view * model * vec4(position, 1.0);
        gl_Position = projection * view_position;
        view_normal = (view * model * vec4(normal, 0.0)).xyz;
        z_view = view_position.z;
    }
);

char *GEOMETRY_FS = GLSL150(
    in float z_view;
    in vec3 view_normal;
    uniform float z_far;
    uniform float z_near;
    out vec4 f_color;
    void main()
    {
        vec3 n = normalize(view_normal);
        float z_norm = (z_view + z_near) / (z_near - z_far);
        f_color = vec4(vec3(0.5) + 0.5 * n, z_norm);
    }
);

char *RECONSTRUCT_VS = GLSL150(
    in vec2 position;
    out vec2 v_position;
    void main()
    {
        gl_Position = vec4(position, 0.0, 1.0);
        v_position = position;
    }
);

char *RECONSTRUCT_FS = GLSL150(
    in vec2 v_position;
    uniform sampler2D channel;
    uniform mat4 projection;
    uniform float z_far;
    uniform float z_near;
    uniform vec3 light_position;
    uniform vec3 light_color;
    uniform float light_radius;
    out vec4 f_color;
    void main()
    {
        vec2 texel = vec2(0.5) + 0.5 * v_position;
        vec4 sample = texture(channel, texel);
        if (sample.w >= 0.999)
            discard;
        float z_norm = sample.w;
        float z_view = (z_near - z_far) * z_norm - z_near;
        float x_ndc = v_position.x;
        float y_ndc = v_position.y;

        // TODO: Easier way to do this?
        float x_view = -z_view * x_ndc / projection[0].x;
        float y_view = -z_view * y_ndc / projection[1].y;

        vec3 position = vec3(x_view, y_view, z_view);
        vec3 n = vec3(-1.0) + 2.0 * sample.xyz;
        vec3 l = normalize(light_position - position);

        // Additive diffuse lighting
        float r = length(light_position - position);
        float attenuation = smoothstep(light_radius, 0.0, r);
        f_color.rgb = max(dot(n, l), 0.0) * light_color * attenuation;
        f_color.a = 1.0;
    }
);

#define NUM_CUBES 16
mat4 cube_models[NUM_CUBES];

#define NUM_LIGHTS 3
struct Lights
{
    vec3 position[NUM_LIGHTS];
    vec3 color[NUM_LIGHTS];
    float radius[NUM_LIGHTS];
} lights;

so_Framebuffer gbuf;
RenderPass geometry_pass;
RenderPass reconstruct_pass;
Mesh cube;
GLuint quad;

void init()
{
    so_make_fbo_rgbad(&gbuf, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA32F);

    RenderPassSource geometry_pass_source = {
        GEOMETRY_VS, GEOMETRY_FS
    };
    geometry_pass_source.from_memory = true;
    geometry_pass = make_render_pass(geometry_pass_source);

    RenderPassSource reconstruct_pass_source = {
        RECONSTRUCT_VS, RECONSTRUCT_FS
    };
    reconstruct_pass_source.from_memory = true;
    reconstruct_pass = make_render_pass(reconstruct_pass_source);

    cube = make_cube();
    quad = make_quad();

    for (u32 i = 0; i < NUM_CUBES-1; i++)
    {
        float x = (-1.0f + 2.0f * frand()) * 2.0f;
        float y = (-1.0f + 2.0f * frand()) * 0.2f;
        float z = +2.0f - 8.0f * frand();
        float s = 0.3f + (0.6f - 0.3f) * frand();
        cube_models[i] = mat_translate(x, y, z) * mat_scale(s) * mat_rotate_x(x) * mat_rotate_y(y);
    }
    cube_models[NUM_CUBES-1] = mat_translate(0.0f, -0.5f, 0.0f) * mat_scale(8.0f, 0.1f, 8.0f);
}

void tick(Input io, float t, float dt)
{
    lights.position[0] = vec3(0.0f, 2.0f, 0.0f);
    lights.position[1] = vec3(sin(t), 0.5f, cos(t));
    lights.position[2] = vec3(0.0f, 1.0f, 3.0f);
    lights.color[0] = vec3(0.93, 0.74, 0.58) * 0.8f;
    lights.color[1] = vec3(0.68, 0.87, 0.91) * 0.7f;
    lights.color[2] = vec3(1.0, 0.55, 0.2) * 0.3f;
    lights.radius[0] = 10.0f;
    lights.radius[1] = 0.5f;
    lights.radius[2] = 4.0f;

    float z_near = 1.0f;
    float z_far = 10.0f;
    mat4 projection = mat_perspective(PI / 4.0f, WINDOW_WIDTH, WINDOW_HEIGHT, z_near, z_far);
    mat4 view = mat_translate(0.0f, 0.0f, -5.0f) *
                mat_rotate_x(0.6f + 0.05f * sin(t)) *
                mat_rotate_y(0.4f + 0.04f * cos(t));

    depth_test(true, GL_LEQUAL);
    depth_write(true);
    glBindFramebuffer(GL_FRAMEBUFFER, gbuf.fbo);
    blend_mode(false);
    clear(0.0f, 0.0f, 0.0f, 1.0f, 1.0f);
    begin(&geometry_pass);
    glBindBuffer(GL_ARRAY_BUFFER, cube.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.ibo);
    attribfv("position", 3, 6, 0);
    attribfv("normal", 3, 6, 3);

    uniformf("projection", projection);
    uniformf("z_near", z_near);
    uniformf("z_far", z_far);
    uniformf("view", view);

    for (u32 i = 0; i < 16; i++)
    {
        uniformf("model", cube_models[i]);
        glDrawElements(GL_TRIANGLES, cube.index_count, cube.index_type, 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    depth_test(false, GL_LEQUAL);
    depth_write(false);
    blend_mode(true, GL_ONE, GL_ONE, GL_FUNC_ADD);
    clearc(0.0f, 0.0f, 0.0f, 1.0f);
    begin(&reconstruct_pass);
    uniformf("projection", projection);
    uniformf("z_near", z_near);
    uniformf("z_far", z_far);
    glBindTexture(GL_TEXTURE_2D, gbuf.color[0]);
    uniformi("channel", 0);
    glBindBuffer(GL_ARRAY_BUFFER, quad);
    attribfv("position", 2, 2, 0);

    for (u32 i = 0; i < NUM_LIGHTS; i++)
    {
        uniformf("light_position", (view * vec4(lights.position[i], 1.0f)).xyz());
        uniformf("light_color", lights.color[i]);
        uniformf("light_radius", lights.radius[i]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

#include "sumo.cpp"
