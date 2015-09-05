/* so_texture - v0.04
Simplified OpenGL texture loading

Changelog
=========
22. august 2015
    Added ability to specify data type and format when loading 2d texture.
    Better error handling.

22. july 2015
    Made so_make_tex2d public.
    Added so_load_png_from_memory.

How to compile
=======================================================================
This file contains both the header file and the implementation file.
To create the implementation in one source file include this header:

    #define SO_TEXTURE_IMPLEMENTATION
    #include "so_texture.h"

Make sure that OpenGL symbols are defined before inclusion of the
header.
*/
#ifndef SO_TEXTURE_HEADER_INCLUDE
#define SO_TEXTURE_HEADER_INCLUDE

extern GLuint
so_load_tex2d(char *path,
              int *out_width = 0,
              int *out_height = 0,
              GLenum min_filter = GL_LINEAR,
              GLenum mag_filter = GL_LINEAR,
              GLenum wrap_s = GL_CLAMP_TO_EDGE,
              GLenum wrap_t = GL_CLAMP_TO_EDGE,
              GLenum internal_format = GL_RGBA,
              GLenum data_type = GL_UNSIGNED_BYTE,
              GLenum data_format = GL_RGBA);

extern GLuint
so_make_tex2d(void *data,
              int width, int height,
              GLenum internal_format,
              GLenum data_format,
              GLenum data_type,
              GLenum min_filter,
              GLenum mag_filter,
              GLenum wrap_s,
              GLenum wrap_t);

extern GLuint
so_load_png_from_memory(const void *png_data,
                        int png_size,
                        GLenum min_filter,
                        GLenum mag_filter,
                        GLenum wrap_s,
                        GLenum wrap_t);

#endif // SO_TEXTURE_HEADER_INCLUDE
#ifdef SO_TEXTURE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define SO_FORCE_LOAD_CHANNELS 4

const char *soi_gl_error_message(GLenum error)
{
    switch (error)
    {
    case 0:      return "No error";
    case 0x0500: return "Invalid enum";
    case 0x0501: return "Invalid value";
    case 0x0502: return "Invalid operation";
    case 0x0503: return "Stack overflow";
    case 0x0504: return "Stack underflow";
    case 0x0505: return "Out of memory";
    case 0x0506: return "Invalid framebuffer operation";
    default:     return "Unknown";
    }
}

GLuint
so_make_tex2d(void *data,
              int width, int height,
              GLenum internal_format,
              GLenum data_format,
              GLenum data_type,
              GLenum min_filter,
              GLenum mag_filter,
              GLenum wrap_s,
              GLenum wrap_t)
{
    GLuint result = 0;
    glGenTextures(1, &result);
    glBindTexture(GL_TEXTURE_2D, result);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, data_format, data_type, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        const char *reason = soi_gl_error_message(error);
        printf("Failed to make 2D texture: %s\n", reason);
        result = 0;
    }
    return result;
}

GLuint
so_load_tex2d(char *path,
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

    GLuint result = so_make_tex2d(data, width, height,
        internal_format, data_format, data_type,
        min_filter, mag_filter, wrap_s, wrap_t);

    if (out_width)    *out_width = width;
    if (out_height)   *out_height = height;
    stbi_image_free(data);
    return result;
}

GLuint
so_load_png_from_memory(const void *png_data,
                        int png_size,
                        GLenum min_filter,
                        GLenum mag_filter,
                        GLenum wrap_s,
                        GLenum wrap_t)
{
    int width, height, channels;
    unsigned char *data = stbi_load_from_memory((const unsigned char*)png_data, png_size, &width, &height, &channels, SO_FORCE_LOAD_CHANNELS);
    if (!data)
    {
        printf("Failed to load png from memory: %s\n", stbi_failure_reason());
        return 0;
    }

    GLuint result = so_make_tex2d(data, width, height, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, min_filter, mag_filter, wrap_s, wrap_t);
    stbi_image_free(data);
    return result;
}

#endif // SO_TEXTURE_IMPLEMENTATION
