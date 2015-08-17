/* so_shader - v0.03
Simplified OpenGL shader loading and uniform- and attribute
configuration.

Changelog
=========
22. july 2015
    Added quick load of vertex/fragment shader from memory

How to compile
=======================================================================
This file contains both the header file and the implementation file.
To create the implementation in one source file include this header:

    #define SO_SHADER_IMPLEMENTATION
    #include "so_shader.h"

Make sure that OpenGL symbols are defined before inclusion of the
header.

#define SOI_ASSERT if you do not want to #include "assert.h".

Interface
=======================================================================
    so_load_shader_vs: Load a vertex and fragment shader
    so_load_shader:    Load a generic number of differently typed shaders
    so_load_shader:    The same, but from raw source code strings

Usage
=======================================================================
Load a shader programme
    char *paths[] = {
        "./data/shader.vs",
        "./data/shader.fs",
        "./data/shader.tcs",
        "./data/shader.tes"
    };
    GLenum types[] = {
        GL_VERTEX_SHADER,
        GL_FRAGMENT_SHADER,
        GL_TESS_CONTROL_SHADER,
        GL_TESS_EVALUATION_SHADER
    };
    GLuint program = so_load_shader(paths, types, 4);
    if (!program) // Failed to load (details have been printf'd)
*/
#ifndef SO_SHADER_HEADER_INCLUDE
#define SO_SHADER_HEADER_INCLUDE

extern GLuint so_load_shader_vf(char *vs_path, char *fs_path);
extern GLuint so_load_shader(char **paths, GLenum *types, int count);
extern GLuint so_load_shader_from_memory(char **srces, GLenum *types, int count);
extern GLuint so_load_shader_vf_from_memory(char *vs_src, char *fs_src);
extern void so_free_shader(GLuint shader);

#ifdef SO_SHADER_IMPLEMENTATION
#ifndef SOI_ASSERT
#include "assert.h"
#define SOI_ASSERT(x) assert(x)
#endif

static bool soi_compile_shader(GLuint shader, GLenum type, char *src)
{
    glShaderSource(shader, 1, (const GLchar**)&src, 0);
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLint length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        char *info = new char[length];
        glGetShaderInfoLog(shader, length, 0, info);
        switch (type)
        {
            case GL_VERTEX_SHADER:
                printf("Failed to compile VERTEX shader:\n%s\n", info);
                break;
            case GL_FRAGMENT_SHADER:
                printf("Failed to compile FRAGMENT shader:\n%s\n", info);
                break;
            default:
                printf("Failed to compile shader:\n%s\n", info);
                break;
        }
        delete[] info;
        return false;
    }
    return true;
}

static GLuint soi_link_program(GLuint *shaders, int count)
{
    GLuint program = glCreateProgram();

    for (int i = 0; i < count; i++)
        glAttachShader(program, shaders[i]);

    glLinkProgram(program);

    for (int i = 0; i < count; i++)
    {
        glDetachShader(program, shaders[i]);
        glDeleteShader(shaders[i]);
    }

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GLint length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        char *info = new char[length];
        glGetProgramInfoLog(program, length, NULL, info);
        printf("Failed to link program: %s\n", info);
        delete[] info;
        return 0;
    }

    return program;
}

GLuint so_load_shader(char **paths, GLenum *types, int count)
{
    char **srces = new char*[count];
    for (int i = 0; i < count; i++)
    {
        FILE *f = fopen(paths[i], "rb");
        if (!f)
        {
            printf("Failed to load shaders: Could not open file %s\n", paths[i]);
            return 0;
        }
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        rewind(f);
        srces[i] = new char[size + 1];
        if (!srces[i])
        {
            printf("Failed to load shaders: File too large %s\n", paths[i]);
            return 0;
        }
        if (!fread(srces[i], 1, size, f))
        {
            printf("Failed to load shaders: Could not read file %s\n", paths[i]);
            return 0;
        }
        srces[i][size] = '\0';
    }
    GLuint result = so_load_shader_from_memory(srces, types, count);
    for (int i = 0; i < count; i++)
        delete[] srces[i];
    delete[] srces;
    return result;
}

GLuint so_load_shader_from_memory(char **srces, GLenum *types, int count)
{
    GLuint shaders[5];
    SOI_ASSERT(count < 5);
    for (int i = 0; i < count; i++)
    {
        shaders[i] = glCreateShader(types[i]);
        if (!soi_compile_shader(shaders[i], types[i], srces[i]))
            return 0;
    }

    return soi_link_program(shaders, count);
}

GLuint so_load_shader_vf_from_memory(char *vs_src, char *fs_src)
{
    char *srces[] = { vs_src, fs_src };
    GLenum types[] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
    return so_load_shader_from_memory(srces, types, 2);
}

void so_free_shader(GLuint shader)
{
    glUseProgram(0);
    glDeleteProgram(shader);
}

GLuint so_load_shader_vf(char *vs_path, char *fs_path)
{
    char *paths[] = { vs_path, fs_path };
    GLenum types[] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
    return so_load_shader(paths, types, 2);
}

#endif SO_SHADER_IMPLEMENTATION
#endif SO_SHADER_HEADER_INCLUDE
