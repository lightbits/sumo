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

void attribv(char *name, GLenum type, u32 count, u32 stride, u32 offset, bool normalized)
{
    GLint location = get_attrib_location(_active_render_pass, name);
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, count, type, normalized, stride, (const GLvoid*)offset);
}

void attribfv(char *name, u32 count, u32 stride, u32 offset, bool normalized)
{
    attribv(name, GL_FLOAT, count, stride * sizeof(GLfloat), offset * sizeof(GLfloat));
}

struct render_pass_source_t
{
    char *vertex_shader;
    char *fragment_shader;
    char *geometry_shader;
    char *tess_ctrl_shader;
    char *tess_eval_shader;
};

void make_render_pass(RenderPass *result, RenderPassSource source)
{
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
    so_free_shader(result->shader);
    map_free(&result->uniforms);
    map_free(&result->attribs);
    if (source.from_memory)
        result->shader = so_load_shader_from_memory(sources, types, count);
    else
        result->shader = so_load_shader(sources, types, count);
    map_alloc(&result->uniforms, 128);
    map_alloc(&result->attribs, 128);
}

RenderPass make_render_pass(RenderPassSource source)
{
    RenderPass result = {};
    make_render_pass(&result, source);
    return result;
}
