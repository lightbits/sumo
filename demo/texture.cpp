#include "sumo.h"
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 640
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS
#define GLSL150(src) "#version 150\n" #src

char *SHADER_VS = GLSL150(
    in vec2 position;
    out vec2 v_texel;
    void main()
    {
        v_texel = vec2(0.5) + 0.5 * vec2(position.x, -position.y);
        gl_Position = vec4(position, 0.0, 1.0);
    }
);

char *SHADER_FS = GLSL150(
    in vec2 v_texel;
    uniform sampler2D channel0;
    uniform sampler2D channel1;
    out vec4 f_color;
    void main()
    {
        vec4 a = texture(channel0, v_texel);
        vec4 b = texture(channel1, v_texel);
        f_color = max(a, b);
    }
);

RenderPass pass;
GLuint channel0;
GLuint channel1;
GLuint quad;

GLuint
so_load_cubemap(char *path,
              int *out_width,
              int *out_height,
              GLenum min_filter,
              GLenum mag_filter,
              GLenum wrap_s,
              GLenum wrap_t,
              GLenum internal_format,
              GLenum data_type,
              GLenum data_format)
{
    int width, height, channels;
    unsigned char *data = stbi_load(path, &width, &height, &channels, SO_FORCE_LOAD_CHANNELS);
    if (!data)
    {
        printf("Failed to load texture (%s): %s\n", path, stbi_failure_reason());
        return 0;
    }

    glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, height / 4);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, width / 3);
    GLuint result = so_make_tex2d(data,
        width / 3, height / 4,
        internal_format, data_format, data_type,
        min_filter, mag_filter, wrap_s, wrap_t);

    if (out_width)    *out_width = width;
    if (out_height)   *out_height = height;
    stbi_image_free(data);
    return result;
}

void init()
{
    RenderPassSource source = {
        SHADER_VS,
        SHADER_FS
    };
    source.from_memory = true;
    pass = make_render_pass(source);
    quad = make_quad();
    float c[] = {
        0.00f, 0.63f, 0.69f, 1.0f,
        0.92f, 0.41f, 0.25f, 1.0f,
        0.42f, 0.29f, 0.24f, 1.0f,
        0.79f, 0.20f, 0.25f, 1.0f
    };
    channel0 = so_make_tex2d(c,
               2, 2,
               GL_RGBA32F,
               GL_RGBA,
               GL_FLOAT,
               GL_NEAREST,
               GL_NEAREST,
               GL_CLAMP_TO_EDGE,
               GL_CLAMP_TO_EDGE);
    channel1 = so_load_tex2d("./assets/hero_sheet.png", 0, 0,
                             GL_NEAREST, GL_NEAREST);
}

void tick(Input io, float t, float dt)
{
    clearc(0.35f, 0.55f, 1.0f, 1.0f);
    begin(&pass);
    blend_mode(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, channel0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, channel1);
    glBindBuffer(GL_ARRAY_BUFFER, quad);
    attribfv("position", 2, 2, 0);
    uniformi("channel0", 0);
    uniformi("channel1", 1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

#include "sumo.cpp"