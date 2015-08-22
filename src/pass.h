#ifndef _pass_h_
#define _pass_h_

typedef Map(GLint) Map_GLint;
struct RenderPass
{
    GLuint shader;
    // GLuint fbo;
    // GLuint textures[...];
    // GLenum texture_types[...]

    Map_GLint uniforms;
    Map_GLint attribs;
};

struct RenderPassSource
{
    char *vertex_shader;
    char *fragment_shader;
    char *geometry_shader;
    char *tess_ctrl_shader;
    char *tess_eval_shader;
    bool from_memory;
};

RenderPass make_render_pass(RenderPassSource source);
void begin(RenderPass *pass);
void attribfv(char *name, u32 count, u32 stride, u32 offset, bool normalized = false);
void attribv(char *name, GLenum type, u32 count, u32 stride, u32 offset, bool normalized = false);
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

#endif
