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
