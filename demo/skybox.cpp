#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS
#define GLSL150(src) "#version 150\n" #src

char *SHADER_VS = GLSL150(
    in vec3 position;
    uniform mat4 projection;
    uniform mat4 view;
    out vec3 v_texel;
    void main()
    {
        v_texel = position;
        vec4 viewpos = view * vec4(4.0 * position, 0.0);
        viewpos.w = 1.0;
        gl_Position = projection * viewpos;
    }
);

char *SHADER_FS = GLSL150(
    in vec3 v_texel;
    uniform samplerCube skybox;
    out vec4 f_color;
    void main()
    {
        f_color = texture(skybox, v_texel);
        float x = abs(v_texel.x);
        float y = abs(v_texel.y);
        float z = abs(v_texel.z);
        if (x > y && x > z)
            f_color.rgb *= vec3(0.5) + 0.5 * vec3(v_texel.x, 0.0, 0.0);
        if (y > x && y > z)
            f_color.rgb *= vec3(0.5) + 0.5 * vec3(0.0, v_texel.y, 0.0);
        if (z > y && z > x)
            f_color.rgb *= vec3(0.5) + 0.5 * vec3(0.0, 0.0, v_texel.z);
    }
);

RenderPass pass;
Mesh cube;
GLuint skybox;
GLuint quad;

void init()
{
    RenderPassSource source = {
        SHADER_VS,
        SHADER_FS
    };
    source.from_memory = true;
    pass = make_render_pass(source);
    cube = make_cube();
    skybox = load_cubemap("./assets/uffizi_cross.hdr",
                          CubemapCrossTB,
                          GL_LINEAR,
                          GL_LINEAR,
                          GL_CLAMP_TO_EDGE,
                          GL_CLAMP_TO_EDGE,
                          GL_CLAMP_TO_EDGE,
                          GL_RGBA32F,
                          GL_UNSIGNED_BYTE,
                          GL_RGBA);
}

void tick(Input io, float t, float dt)
{
    mat4 projection = mat_perspective(PI / 4.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.1f, 10.0f);
    // mat4 view = mat_translate(0.0f, 0.0f, -5.0f) * mat_rotate_y(0.3f * t);

    mat4 view = camera_holdclick(io, dt);

    depth_test(true, GL_LEQUAL);
    depth_write(true);
    clear(0.35f, 0.55f, 1.0f, 1.0f, 1.0f);
    begin(&pass);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
    glBindBuffer(GL_ARRAY_BUFFER, cube.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.ibo);
    attribfv("position", 3, 6, 0);
    uniformf("projection", projection);
    uniformf("view", view);
    uniformi("skybox", 0);
    glDrawElements(GL_TRIANGLES, cube.index_count, cube.index_type, 0);
}

#include "sumo.cpp"
