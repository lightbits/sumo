#ifndef SM_CUBEMAP_HEADER_INCLUDE
#define SM_CUBEMAP_HEADER_INCLUDE

enum CubemapFormat
{
    CubemapCrossLR,
    CubemapCrossTB
};

GLuint load_cubemap(char *path,
                    CubemapFormat format,
                    GLenum min_filter,
                    GLenum mag_filter,
                    GLenum wrap_r,
                    GLenum wrap_s,
                    GLenum wrap_t,
                    GLenum internal_format,
                    GLenum data_type,
                    GLenum data_format);

#endif // SM_CUBEMAP_HEADER_INCLUDE
#ifdef SM_CUBEMAP_IMPLEMENTATION

GLuint load_cubemap(char *path,
                    CubemapFormat format,
                    GLenum min_filter,
                    GLenum mag_filter,
                    GLenum wrap_r,
                    GLenum wrap_s,
                    GLenum wrap_t,
                    GLenum internal_format,
                    GLenum data_type,
                    GLenum data_format)
{
    int width, height, channels;
    void *data = 0;
    if (data_type == GL_FLOAT)
        data = (void*)stbi_loadf(path, &width, &height, &channels, SO_FORCE_LOAD_CHANNELS);
    else
        data = (void*)stbi_load(path, &width, &height, &channels, SO_FORCE_LOAD_CHANNELS);
    if (!data)
    {
        printf("Failed to load texture (%s): %s\n", path, stbi_failure_reason());
        return 0;
    }

    int fh = height / 4;
    int fw = width / 3;

    GLuint result = 0;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &result);
    glBindTexture(GL_TEXTURE_CUBE_MAP, result);

    #define DoFace(face, x, y) \
            glPixelStorei(GL_UNPACK_SKIP_ROWS, y * fh); \
            glPixelStorei(GL_UNPACK_SKIP_PIXELS, x * fw); \
            glTexImage2D(face, 0, internal_format, fw, fh, 0, data_format, data_type, data);

    glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
    DoFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, 1);
    DoFace(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 2, 1);
    DoFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 1, 2);
    DoFace(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 1, 0);
    DoFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 1, 3);
    DoFace(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 1, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    #undef Face

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrap_r);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrap_s);
    // glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    stbi_image_free(data);
    return result;
}
#endif // SM_CUBEMAP_IMPLEMENTATION
