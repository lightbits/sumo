#ifndef SM_PASS_HEADER_INCLUDE
#define SM_PASS_HEADER_INCLUDE

typedef Map(GLint) Map_GLint;
struct RenderPass
{
    GLuint shader;
    Map_GLint uniforms;
    Map_GLint attribs;
};

// Create a render pass from pointers to source code in memory
RenderPass make_render_pass(char *vertex_shader = 0,
                            char *fragment_shader = 0,
                            char *geometry_shader = 0,
                            char *tess_ctrl_shader = 0,
                            char *tess_eval_shader = 0);

// Load a render pass from filenames of shaders on disk
RenderPass load_render_pass(char *vertex_shader = 0,
                            char *fragment_shader = 0,
                            char *geometry_shader = 0,
                            char *tess_ctrl_shader = 0,
                            char *tess_eval_shader = 0);

void begin(RenderPass *pass);
void attribfv(char *name, u32 count, u32 stride, u32 offset, bool normalized = false);
void attribv(char *name, GLenum type, u32 count, u32 stride, void *offset, bool normalized = false);
void attribdiv(char *name, u32 divisor);
void uniformi(char *name, s32 x);
void uniformi(char *name, s32 x, s32 y);
void uniformi(char *name, s32 x, s32 y, s32 z);
void uniformi(char *name, s32 x, s32 y, s32 z, s32 w);
void uniformf(char *name, float x);
void uniformf(char *name, float x, float y);
void uniformf(char *name, float x, float y, float z);
void uniformf(char *name, float x, float y, float z, float w);
void uniformf(char *name, vec2 x);
void uniformf(char *name, vec3 x);
void uniformf(char *name, vec4 x);
void uniformf(char *name, mat4 x);

#endif // SM_PASS_HEADER_INCLUDE
#ifdef SM_PASS_IMPLEMENTATION

RenderPass *_active_render_pass;
void begin(RenderPass *pass)
{
    _active_render_pass = pass;
    glUseProgram(pass->shader);
}

// void end(RenderPass *pass)
// {
//     // deferred something
//     glUseProgram(...)
//     ...
// }

GLint get_attrib_location(RenderPass *pass, char *name)
{
    GLint result = -1;
    GLint *locationp = map_get(&pass->attribs, name, GLint);
    if (!locationp)
    {
        result = glGetAttribLocation(pass->shader, name);
        map_set(&pass->attribs, name, result);
    }
    else
    {
        result = *locationp;
    }
    if (result < 0)
        printf("Unused or non-existent attribute %s\n", name);
    ASSERT(result >= 0);
    return result;
}

GLint get_uniform_location(RenderPass *pass, char *name)
{
    GLint result = -1;
    GLint *locationp = map_get(&pass->uniforms, name, GLint);
    if (!locationp)
    {
        result = glGetUniformLocation(pass->shader, name);
        map_set(&pass->uniforms, name, result);
    }
    else
    {
        result = *locationp;
    }
    if (result < 0)
        printf("Unused or non-existent uniform %s\n", name);
    ASSERT(result >= 0);
    return result;
}

void uniformi(char *name, s32 x)
{
    GLint location = get_uniform_location(_active_render_pass, name);
    glUniform1i(location, x);
}

void uniformi(char *name, s32 x, s32 y)
{
    GLint location = get_uniform_location(_active_render_pass, name);
    glUniform1i(location, x);
}

void uniformi(char *name, s32 x, s32 y, s32 z)
{
    GLint location = get_uniform_location(_active_render_pass, name);
    glUniform1i(location, x);
}

void uniformi(char *name, s32 x, s32 y, s32 z, s32 w)
{
    GLint location = get_uniform_location(_active_render_pass, name);
    glUniform1i(location, x);
}

void uniformf(char *name, float x)
{
    GLint location = get_uniform_location(_active_render_pass, name);
    glUniform1f(location, x);
}

void uniformf(char *name, float x, float y)
{
    GLint location = get_uniform_location(_active_render_pass, name);
    glUniform1f(location, x);
}

void uniformf(char *name, float x, float y, float z)
{
    GLint location = get_uniform_location(_active_render_pass, name);
    glUniform1f(location, x);
}

void uniformf(char *name, float x, float y, float z, float w)
{
    GLint location = get_uniform_location(_active_render_pass, name);
    glUniform1f(location, x);
}

void uniformf(char *name, vec2 x)
{
    GLint location = get_uniform_location(_active_render_pass, name);
    glUniform2f(location, x.x, x.y);
}

void uniformf(char *name, vec3 x)
{
    GLint location = get_uniform_location(_active_render_pass, name);
    glUniform3f(location, x.x, x.y, x.z);
}

void uniformf(char *name, vec4 x)
{
    GLint location = get_uniform_location(_active_render_pass, name);
    glUniform4f(location, x.x, x.y, x.z, x.w);
}

void uniformf(char *name, mat4 x)
{
    GLint location = get_uniform_location(_active_render_pass, name);
    glUniformMatrix4fv(location, 1, GL_FALSE, x.data);
}

void attribv(char *name, GLenum type, u32 count, u32 stride, void *offset, bool normalized)
{
    GLint location = get_attrib_location(_active_render_pass, name);
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, count, type, normalized, stride, (const GLvoid*)offset);
}

void attribfv(char *name, u32 count, u32 stride, u32 offset, bool normalized)
{
    attribv(name, GL_FLOAT, count, stride * sizeof(GLfloat), (void*)(offset * sizeof(GLfloat)));
}

void attribdiv(char *name, u32 divisor)
{
    GLint location = get_attrib_location(_active_render_pass, name);
    glVertexAttribDivisor(location, divisor);
}

RenderPass make_render_pass(char *vertex_shader,
                            char *fragment_shader,
                            char *geometry_shader,
                            char *tess_ctrl_shader,
                            char *tess_eval_shader,
                            bool from_memory)
{
    RenderPass result = {};
    char *sources[5];
    GLenum types[5];
    u32 count = 0;
    if (vertex_shader)
    {
        sources[count] = vertex_shader;
        types[count++] = GL_VERTEX_SHADER;
    }
    if (fragment_shader)
    {
        sources[count] = fragment_shader;
        types[count++] = GL_FRAGMENT_SHADER;
    }
    if (geometry_shader)
    {
        sources[count] = geometry_shader;
        types[count++] = GL_GEOMETRY_SHADER;
    }
    if (tess_ctrl_shader)
    {
        sources[count] = tess_ctrl_shader;
        types[count++] = GL_TESS_CONTROL_SHADER;
    }
    if (tess_eval_shader)
    {
        sources[count] = tess_eval_shader;
        types[count++] = GL_TESS_EVALUATION_SHADER;
    }
    so_free_shader(result.shader);
    map_free(&result.uniforms);
    map_free(&result.attribs);
    if (from_memory)
        result.shader = so_load_shader_from_memory(sources, types, count);
    else
        result.shader = so_load_shader(sources, types, count);
    map_alloc(&result.uniforms, 128);
    map_alloc(&result.attribs, 128);
    return result;
}

RenderPass make_render_pass(char *vs, char *fs, char *gs, char *tcs, char *tes)
{
    return make_render_pass(vs, fs, gs, tcs, tes, true);
}

RenderPass load_render_pass(char *vs, char *fs, char *gs, char *tcs, char *tes)
{
    return make_render_pass(vs, fs, gs, tcs, tes, false);
}

#endif
