#include "prototype.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

typedef sorted_array_t(GLint) sorted_GLint_array_t;
struct render_pass_t
{
    GLuint shader;
    // GLuint fbo;
    // GLuint textures[...];
    // GLenum texture_types[...]

    sorted_GLint_array_t uniforms;
    sorted_GLint_array_t attribs;
};

render_pass_t *_active_render_pass;
void begin(render_pass_t *pass)
{
    _active_render_pass = pass;
    glUseProgram(pass->shader);
}

// void end(render_pass_t *pass)
// {
//     // deferred something
//     glUseProgram(...)
//     ...
// }

GLint
get_attrib_location(render_pass_t *pass, char *name)
{
    GLuint result = -1;
    GLint *locationp = sorted_array_get(&pass->attribs, name, GLint);
    if (!locationp)
    {
        result = glGetAttribLocation(pass->shader, name);
        sorted_array_set(&pass->attribs, name, result);
    }
    else
    {
        result = *locationp;
    }
    return result;
}

GLint
get_uniform_location(render_pass_t *pass, char *name)
{
    GLuint result = -1;
    GLint *locationp = sorted_array_get(&pass->uniforms, name, GLint);
    if (!locationp)
    {
        result = glGetUniformLocation(pass->shader, name);
        sorted_array_set(&pass->uniforms, name, result);
    }
    else
    {
        result = *locationp;
    }
    return result;
}

void
uniform(char *name, float x)
{
    GLint location = get_uniform_location(_active_render_pass, name);
    glUniform1f(location, x);
}

void
uniform(char *name, float x, float y)
{
    GLint location = get_uniform_location(_active_render_pass, name);
    glUniform1f(location, x);
}

void
uniform(char *name, float x, float y, float z)
{
    GLint location = get_uniform_location(_active_render_pass, name);
    glUniform1f(location, x);
}

void
uniform(char *name, float x, float y, float z, float w)
{
    GLint location = get_uniform_location(_active_render_pass, name);
    glUniform1f(location, x);
}

void
uniform(char *name, vec2 x)
{
    GLint location = get_uniform_location(_active_render_pass, name);
    glUniform2f(location, x.x, x.y);
}

void
uniform(char *name, vec3 x)
{
    GLint location = get_uniform_location(_active_render_pass, name);
    glUniform3f(location, x.x, x.y, x.z);
}

void
uniform(char *name, vec4 x)
{
    GLint location = get_uniform_location(_active_render_pass, name);
    glUniform4f(location, x.x, x.y, x.z, x.w);
}

void
uniform(char *name, mat4 x)
{
    GLint location = get_uniform_location(_active_render_pass, name);
    glUniformMatrix4fv(location, 1, GL_FALSE, x.data);
}

void
attribv(char *name,
        GLenum type,
        u32 count,
        u32 stride,
        u32 offset,
        bool normalized = false)
{
    // TODO: This is a bug (and applies for all above calls to uniform)
    // The pointer to name goes stale, since it is a local parameter.
    // But we insert the pointer itself in the sorted array. Instead
    // we should copy the string, and be able to store strings of
    // varied lengths somehow... Oh boy.
    GLint location = get_attrib_location(_active_render_pass, name);
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location,
                          count,
                          type,
                          normalized,
                          stride,
                          (const GLvoid*)offset);
}

struct render_pass_source_t
{
    char *vertex_shader;
    char *fragment_shader;
    char *geometry_shader;
    char *tess_ctrl_shader;
    char *tess_eval_shader;
};

render_pass_t make_render_pass(render_pass_source_t source, bool from_memory)
{
    render_pass_t result = {};
    char *sources[5];
    GLenum types[5];
    u32 count = 0;
    if (source.vertex_shader)
    {
        sources[count] = source.vertex_shader;
        types[count++] = GL_VERTEX_SHADER;
    }
    if (source.fragment_shader)
    {
        sources[count] = source.fragment_shader;
        types[count++] = GL_FRAGMENT_SHADER;
    }
    if (source.geometry_shader)
    {
        sources[count] = source.geometry_shader;
        types[count++] = GL_GEOMETRY_SHADER;
    }
    if (source.tess_ctrl_shader)
    {
        sources[count] = source.tess_ctrl_shader;
        types[count++] = GL_TESS_CONTROL_SHADER;
    }
    if (source.tess_eval_shader)
    {
        sources[count] = source.tess_eval_shader;
        types[count++] = GL_TESS_EVALUATION_SHADER;
    }
    if (from_memory)
        result.shader = so_load_shader_from_memory(sources, types, count);
    else
        result.shader = so_load_shader(sources, types, count);

    sorted_array_alloc(&result.uniforms, 128);
    sorted_array_alloc(&result.attribs, 128);
    return result;
}

render_pass_t pass1;

void init()
{
    // char *keys[16];
    // float data[16];
    sorted_r32_array_t a = {};
    sorted_array_alloc(&a, 16);
    sorted_array_set(&a, "acb", 4.4f);
    sorted_array_set(&a, "ab", 3.3f);
    sorted_array_set(&a, "aa", 2.2f);
    sorted_array_set(&a, "a", 1.1f);

    for (int i = 0; i < sorted_array_len(&a); i++)
    {
        char *key = a.base.keys[i];
        float *x = sorted_array_get(&a, key, float);
        if (x)
            printf("%d : %s = %f\n", i, key, *x);
    }

    render_pass_source_t pass1_src = {
        "shaders/hello.vs",
        "shaders/hello.fs"
    };

    pass1 = make_render_pass(pass1_src, false);
}

void tick(float t, float dt)
{
    begin(&pass1);
    attribv("position", GL_FLOAT, 2, 2 * sizeof(GLfloat), 0);

    Clearc(0.35f, 0.55f, 1.0f, 1.0f);
    ImGui::NewFrame();
    static float lightColor[4];
    static float attenuation;
    ImGui::Begin("Diffuse Shader");
    ImGui::ColorEdit4("lightColor", lightColor);
    ImGui::SliderFloat("attenuation", &attenuation, 1.0f, 16.0f);
    ImGui::End();
    ImGui::Render();
}

#include "prototype.cpp"
