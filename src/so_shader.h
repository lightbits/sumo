/* so_shader - v0.05
Simplified OpenGL shader loading and uniform- and attribute
configuration.

Changelog
=========
10. januar 2016
    Revamped API. Can now pass in user-created program handle.
    Useful for pre-link phase operations.

24. august 2015
    Fixed file pointer leak

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
#define SO_MAX_SHADERS 6

extern bool so_compile_shader_from_memory(GLuint shader, GLenum type, char *src);
extern bool so_compile_shader_from_file(GLuint shader, GLenum type, char *path);
extern bool so_link_program(GLuint program, GLuint *shaders, int count);

// program
//     The callee should create this using glCreateProgram prior to calling
//     this function.
// sources[i]
//     Either a filename or the raw source code for a shader.
// types[i]
//     Must be one of
//     GL_COMPUTE_SHADER,
//     GL_VERTEX_SHADER,
//     GL_TESS_CONTROL_SHADER,
//     GL_TESS_EVALUATION_SHADER,
//     GL_GEOMETRY_SHADER, or
//     GL_FRAGMENT_SHADER
// is_file[i]
//     If set to true, sources[i] is treated as a null-terminated filename
//     to a file containing the shader source. If set to false, sources[i]
//     is treated as a raw shader source string.
// count
//     The number of shaders. Cannot exceed 6 items.
// Return
//     false if compilation or linking failed, true if shaders
//     successfully compiled and the program successfully linked.
extern bool so_load_program(GLuint program, char **sources, GLenum *types, bool *is_file, int count);

// Notes
//     This performs the same operations as the previous function, but creates
//     a program handle internally. The above function is more useful if you
//     need to do some pre-linking operations to the program, such as
//     glBindFragDataLocation to specify multiple color outputs.
//
//     The returned handle should be freed with glDeleteProgram(.)
//     once it is no longer needed. Any shaders that are loaded
//     are however internally detached and freed after the linking
//     process.
extern GLuint so_load_program(char **sources, GLenum *types, bool *is_file, int count);

extern bool so_load_program(GLuint program,
                            char *vertex_shader,
                            char *fragment_shader,
                            char *geometry_shader,
                            char *tess_ctrl_shader,
                            char *tess_eval_shader,
                            char *compute_shader,
                            bool *is_file);

extern bool so_load_program_from_files(GLuint program,
                                       char *vertex_shader = 0,
                                       char *fragment_shader = 0,
                                       char *geometry_shader = 0,
                                       char *tess_ctrl_shader = 0,
                                       char *tess_eval_shader = 0,
                                       char *compute_shader = 0);

extern bool so_load_program_from_files(char *vertex_shader = 0,
                                       char *fragment_shader = 0,
                                       char *geometry_shader = 0,
                                       char *tess_ctrl_shader = 0,
                                       char *tess_eval_shader = 0,
                                       char *compute_shader = 0);

extern bool so_load_program_from_memory(GLuint program,
                                        char *vertex_shader = 0,
                                        char *fragment_shader = 0,
                                        char *geometry_shader = 0,
                                        char *tess_ctrl_shader = 0,
                                        char *tess_eval_shader = 0,
                                        char *compute_shader = 0);

extern bool so_load_program_from_memory(char *vertex_shader = 0,
                                        char *fragment_shader = 0,
                                        char *geometry_shader = 0,
                                        char *tess_ctrl_shader = 0,
                                        char *tess_eval_shader = 0,
                                        char *compute_shader = 0);

#endif // SO_SHADER_HEADER_INCLUDE
#ifdef SO_SHADER_IMPLEMENTATION
#ifndef SOI_ASSERT
#include "assert.h"
#define SOI_ASSERT(x) assert(x)
#endif

bool so_compile_shader_from_memory(GLuint shader, GLenum type, char *src)
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

bool so_compile_shader_from_file(GLuint shader, GLenum type, char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f)
    {
        printf("Could not open file %s\n", path);
        return false;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    char *data = new char[size + 1]; // + 1 for null terminator
    if (!data)
    {
        printf("File too large %s\n", path);
        fclose(f);
        return false;
    }
    if (!fread(data, 1, size, f))
    {
        printf("Could not read file %s\n", path);
        delete[] data;
        fclose(f);
        return false;
    }
    data[size] = '\0';
    fclose(f);

    bool compile_status = so_compile_shader_from_memory(shader, type, data);
    delete[] data;
    return compile_status;
}

bool so_link_program(GLuint program, GLuint *shaders, int count)
{
    SOI_ASSERT(program != 0);

    glLinkProgram(program);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE) {
        GLint length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        char *info = new char[length];
        glGetProgramInfoLog(program, length, NULL, info);
        printf("Failed to link program: %s\n", info);
        delete[] info;
        return false;
    }

    return true;
}

bool so_load_program(GLuint program, char **sources, GLenum *types, bool *is_file, int count)
{
    SOI_ASSERT(program != 0);

    GLuint shaders[SO_MAX_SHADERS];
    SOI_ASSERT(count <= SO_MAX_SHADERS);
    for (int i = 0; i < count; i++)
    {
        shaders[i] = glCreateShader(types[i]);
        bool compile_status;
        if (is_file[i])
        {
            compile_status = so_compile_shader_from_file(shaders[i], types[i], sources[i]);
        }
        else
        {
            compile_status = so_compile_shader_from_memory(shaders[i], types[i], sources[i]);
        }
        if (!compile_status)
        {
            for (int j = 0; j <= i; j++)
                glDeleteShader(shaders[i]);
            return false;
        }
    }

    for (int i = 0; i < count; i++)
        glAttachShader(program, shaders[i]);

    if (!so_link_program(program, shaders, count))
    {
        for (int i = 0; i < count; i++)
        {
            glDetachShader(program, shaders[i]);
            glDeleteShader(shaders[i]);
        }
        return false;
    }

    for (int i = 0; i < count; i++)
    {
        glDetachShader(program, shaders[i]);
        glDeleteShader(shaders[i]);
    }

    return true;
}

GLuint so_load_program(char **sources, GLenum *types, bool *is_file, int count)
{
    GLuint program = glCreateProgram();
    if (so_load_program(program, sources, types, is_file, count))
    {
        return program;
    }
    else
    {
        glDeleteProgram(program);
        return 0;
    }
}

bool so_load_program(GLuint program,
                     char *vertex_shader,
                     char *fragment_shader,
                     char *geometry_shader,
                     char *tess_ctrl_shader,
                     char *tess_eval_shader,
                     char *compute_shader,
                     bool *is_file)
{
    GLenum types[SO_MAX_SHADERS];
    char *sources[SO_MAX_SHADERS];
    int count = 0;
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
    if (compute_shader)
    {
        sources[count] = compute_shader;
        types[count++] = GL_COMPUTE_SHADER;
    }
    return so_load_program(program, sources, types, is_file, count);
}

bool so_load_program_from_files(GLuint program,
                                char *vs, char *fs, char *gs, char *tcs, char *tes, char *cs)
{
    bool is_file[SO_MAX_SHADERS];
    for (int i = 0; i < SO_MAX_SHADERS; i++)
        is_file[i] = true;
    return so_load_program(program, vs, fs, gs, tcs, tes, cs, is_file);
}

bool so_load_program_from_files(char *vs, char *fs, char *gs, char *tcs, char *tes, char *cs)
{
    GLuint program = glCreateProgram();
    if (so_load_program_from_files(program, vs, fs, gs, tcs, tes, cs))
    {
        glDeleteProgram(program);
        return false;
    }
    else
    {
        return true;
    }
}

bool so_load_program_from_memory(GLuint program,
                                 char *vs, char *fs, char *gs, char *tcs, char *tes, char *cs)
{
    bool is_file[SO_MAX_SHADERS];
    for (int i = 0; i < SO_MAX_SHADERS; i++)
        is_file[i] = false;
    return so_load_program(program, vs, fs, gs, tcs, tes, cs, is_file);
}

bool so_load_program_from_memory(char *vs, char *fs, char *gs, char *tcs, char *tes, char *cs)
{
    GLuint program = glCreateProgram();
    if (!so_load_program_from_memory(program, vs, fs, gs, tcs, tes, cs))
    {
        glDeleteProgram(program);
        return false;
    }
    else
    {
        return true;
    }
}

#endif // SO_SHADER_IMPLEMENTATION
