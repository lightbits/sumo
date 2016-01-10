#ifndef SM_PASS_HEADER_INCLUDE
#define SM_PASS_HEADER_INCLUDE

typedef Map(GLint) Map_GLint;
struct RenderPass
{
    GLuint program;
    Map_GLint uniforms;
    Map_GLint attribs;
};

// Create a render pass from pointers to source code in memory
RenderPass make_render_pass(char *vertex_shader = 0,
                            char *fragment_shader = 0,
                            char *geometry_shader = 0,
                            char *tess_ctrl_shader = 0,
                            char *tess_eval_shader = 0,
                            char *compute_shader = 0);

// Load a render pass from filenames of shaders on disk
RenderPass load_render_pass(char *vertex_shader = 0,
                            char *fragment_shader = 0,
                            char *geometry_shader = 0,
                            char *tess_ctrl_shader = 0,
                            char *tess_eval_shader = 0,
                            char *compute_shader = 0);

// Create a render pass from pointers to source code in memory
RenderPass make_render_pass(GLuint program,
                            char *vertex_shader = 0,
                            char *fragment_shader = 0,
                            char *geometry_shader = 0,
                            char *tess_ctrl_shader = 0,
                            char *tess_eval_shader = 0,
                            char *compute_shader = 0);

// Load a render pass from filenames of shaders on disk
RenderPass load_render_pass(GLuint program,
                            char *vertex_shader = 0,
                            char *fragment_shader = 0,
                            char *geometry_shader = 0,
                            char *tess_ctrl_shader = 0,
                            char *tess_eval_shader = 0,
                            char *compute_shader = 0);


void begin(RenderPass *pass);
void attribfv(char *name, u32 count, u32 stride, u32 offset, bool normalized = false);
void attribv(char *name, GLenum type, u32 count, u32 stride, u32 offset, bool normalized = false);
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
#ifdef USE_NEW_MATH
void uniformf(char *name, mat2 x);
void uniformf(char *name, mat3 x);
#endif
void uniformf(char *name, mat4 x);

#endif // SM_PASS_HEADER_INCLUDE
#ifdef SM_PASS_IMPLEMENTATION

RenderPass *_active_render_pass;
void begin(RenderPass *pass)
{
    _active_render_pass = pass;
    glUseProgram(pass->program);
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
        result = glGetAttribLocation(pass->program, name);
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
        result = glGetUniformLocation(pass->program, name);
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

#ifdef USE_NEW_MATH
void uniformf(char *name, mat2 x)
{
    GLint location = get_uniform_location(_active_render_pass, name);
    glUniformMatrix2fv(location, 1, GL_FALSE, x.data);
}

void uniformf(char *name, mat3 x)
{
    GLint location = get_uniform_location(_active_render_pass, name);
    glUniformMatrix3fv(location, 1, GL_FALSE, x.data);
}
#endif

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

void attribdiv(char *name, u32 divisor)
{
    GLint location = get_attrib_location(_active_render_pass, name);
    glVertexAttribDivisor(location, divisor);
}

static RenderPass
make_render_pass(GLuint program,
                 char *vs, char *fs,
                 char *gs, char *tcs,
                 char *tes, char *cs,
                 bool from_files)
{
    #if 0
    // TODO: Delete previous renderpass
    glUseProgram(0);
    glDeleteProgram(result.program);
    map_free(&result.uniforms);
    map_free(&result.attribs);
    #endif

    RenderPass result = {};
    result.program = program;
    if (from_files)
    {
        so_load_program_from_files(program, vs, fs, gs, tcs, tes, cs);
    }
    else
    {
        so_load_program_from_memory(program, vs, fs, gs, tcs, tes, cs);
    }
    map_alloc(&result.uniforms, 128);
    map_alloc(&result.attribs, 128);
    return result;
}

// program
//     Generated by glCreateProgram() prior to calling this function.
//     Useful for specifying pre-link stage operations, such as
//     glBindFragDataLocation.
RenderPass make_render_pass(GLuint program, char *vs, char *fs, char *gs, char *tcs, char *tes, char *cs)
{
    return make_render_pass(program, vs, fs, gs, tcs, tes, cs, false);
}

RenderPass make_render_pass(char *vs, char *fs, char *gs, char *tcs, char *tes, char *cs)
{
    GLuint program = glCreateProgram();
    return make_render_pass(program, vs, fs, gs, tcs, tes, cs);
}

RenderPass load_render_pass(GLuint program, char *vs, char *fs, char *gs, char *tcs, char *tes, char *cs)
{
    return make_render_pass(program, vs, fs, gs, tcs, tes, cs, true);
}

RenderPass load_render_pass(char *vs, char *fs, char *gs, char *tcs, char *tes, char *cs)
{
    GLuint program = glCreateProgram();
    return load_render_pass(program, vs, fs, gs, tcs, tes, cs);
}


#endif
