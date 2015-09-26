/* so_mesh - v0.1

Changelog
====================================================================
26. september 2015
    Converted to so code format for code reuse purposes.

How to compile
====================================================================
This file contains both the header file and the implementation file.
To create the implementation in one source file include this header:

    #define SO_MESH_IMPLEMENTATION
    #include "so_mesh.h"

The file expects the following OpenGL symbols to be defined:
    GLuint GLenum GLsizei GLvoid
    glGenBuffers glBindBuffer glBufferData
    GL_ARRAY_BUFFER GL_STATIC_DRAW GL_UNSIGNED_INT
*/

#ifndef SO_MESH_HEADER_INCLUDE
#define SO_MESH_HEADER_INCLUDE

struct Mesh
{
    GLuint vbo;
    GLuint ibo;
    unsigned int index_count;
    GLenum index_type;
};

extern GLuint make_buffer(GLenum target, GLsizei size, GLvoid *data, GLenum usage);
extern GLuint make_quad();
extern Mesh   make_cube();

#endif // SO_MESH_HEADER_INCLUDE
#ifdef SO_MESH_IMPLEMENTATION

GLuint make_buffer(GLenum target, GLsizei size, GLvoid *data, GLenum usage)
{
    GLuint result = 0;
    glGenBuffers(1, &result);
    glBindBuffer(target, result);
    glBufferData(target, size, data, usage);
    glBindBuffer(target, 0);
    return result;
}

GLuint make_quad()
{
    GLfloat v[] = {
        -1.0f, -1.0f,
        +1.0f, -1.0f,
        +1.0f, +1.0f,
        +1.0f, +1.0f,
        -1.0f, +1.0f,
        -1.0f, -1.0f
    };
    return make_buffer(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
}

Mesh make_cube()
{
    GLfloat v[] = {
        // Front
        -1.0f, -1.0f, +1.0f, 0.0f, 0.0f, 1.0f,
        +1.0f, -1.0f, +1.0f, 0.0f, 0.0f, 1.0f,
        +1.0f, +1.0f, +1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, +1.0f, +1.0f, 0.0f, 0.0f, 1.0f,

        // Back
        +1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
        -1.0f, +1.0f, -1.0f, 0.0f, 0.0f, -1.0f,
        +1.0f, +1.0f, -1.0f, 0.0f, 0.0f, -1.0f,

        // Left
        -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, +1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, +1.0f, +1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, +1.0f, -1.0f, -1.0f, 0.0f, 0.0f,

        // Right
        +1.0f, -1.0f, +1.0f, 1.0f, 0.0f, 0.0f,
        +1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
        +1.0f, +1.0f, -1.0f, 1.0f, 0.0f, 0.0f,
        +1.0f, +1.0f, +1.0f, 1.0f, 0.0f, 0.0f,

        // Top
        -1.0f, +1.0f, +1.0f, 0.0f, 1.0f, 0.0f,
        +1.0f, +1.0f, +1.0f, 0.0f, 1.0f, 0.0f,
        +1.0f, +1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, +1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

        // Bottom
        +1.0f, -1.0f, +1.0f, 0.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, +1.0f, 0.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f,
        +1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f
    };

    GLuint i[] = {
         0,  1,  2,  2,  3,  0,
         4,  5,  6,  6,  7,  4,
         8,  9, 10, 10, 11,  8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };

    Mesh result = {};
    result.vbo = make_buffer(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
    result.ibo = make_buffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(i), i, GL_STATIC_DRAW);
    result.index_count = 36;
    result.index_type = GL_UNSIGNED_INT;
    return result;
}

#endif // SO_MESH_IMPLEMENTATION
