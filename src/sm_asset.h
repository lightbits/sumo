#ifndef SM_ASSET_HEADER_INCLUDE
#define SM_ASSET_HEADER_INCLUDE

// Mesh loading flags
#define MESH_NORMALIZE (1 << 0)
#define MESH_FLIP_X    (1 << 1)
#define MESH_FLIP_Y    (1 << 2)
#define MESH_FLIP_Z    (1 << 3)

struct MeshAsset
{
    GLuint positions;
    GLuint normals;
    GLuint texels;
    GLuint indices;

    struct Buffer
    {
        char *data;
        r32 *positions;
        r32 *normals;
        r32 *texels;
        u32 *indices;
    } buffer;

    u32 num_positions;
    u32 num_normals;
    u32 num_texels;
    u32 num_indices;
};

MeshAsset load_mesh(char *filename, u32 flags=0);
void load_mesh_from_memory(void  *data,
                           r32  **out_positions,
                           r32  **out_normals,
                           r32  **out_texels,
                           u32  **out_indices,
                           u32   *out_num_positions,
                           u32   *out_num_normals,
                           u32   *out_num_texels,
                           u32   *out_num_indices);

#endif // SM_ASSET_HEADER_INCLUDE
#ifdef SM_ASSET_IMPLEMENTATION

static char *asset_read_u32(char *data, u32 *result)
{
    *result = *(u32*)(data);
    return data + sizeof(u32);
}

static char *asset_read_u32(char *data, u32 **result, u32 count)
{
    *result = (u32*)(data);
    return data + sizeof(u32) * count;
}

static char *asset_read_r32(char *data, r32 **result, u32 count)
{
    *result = (r32*)data;
    return data + sizeof(r32) * count;
}

void load_mesh_from_memory(void  *data,
                           r32  **out_positions,
                           r32  **out_normals,
                           r32  **out_texels,
                           u32  **out_indices,
                           u32   *out_num_positions,
                           u32   *out_num_normals,
                           u32   *out_num_texels,
                           u32   *out_num_indices)
{
    char *read = (char*)data;
    read = asset_read_u32(read, out_num_positions);
    read = asset_read_u32(read, out_num_normals);
    read = asset_read_u32(read, out_num_texels);
    read = asset_read_u32(read, out_num_indices);
    read = asset_read_r32(read, out_positions, *out_num_positions);
    read = asset_read_r32(read, out_normals, *out_num_normals);
    read = asset_read_r32(read, out_texels, *out_num_texels);
    read = asset_read_u32(read, out_indices, *out_num_indices);
}

MeshAsset load_mesh(char *filename, u32 flags)
{
    char *data = so_read_file(filename);

    MeshAsset result = {};
    result.buffer.data = data;
    load_mesh_from_memory(data,
        &result.buffer.positions,
        &result.buffer.normals,
        &result.buffer.texels,
        &result.buffer.indices,
        &result.num_positions,
        &result.num_normals,
        &result.num_texels,
        &result.num_indices);

    if (flags & MESH_NORMALIZE)
    {
        r32 max_position = 0.0f;
        for (u32 i = 0; i < result.num_positions; i++)
        {
            r32 p = abs(result.buffer.positions[i]);
            if (p > max_position)
            {
                max_position = p;
            }
        }

        for (u32 i = 0; i < result.num_positions; i++)
        {
            result.buffer.positions[i] /= max_position;
        }
    }

    if (flags & MESH_FLIP_X)
    {
        // Not implemented yet
        ASSERT(false);
    }

    if (flags & MESH_FLIP_Y)
    {
        // Not implemented yet
        ASSERT(false);
    }

    if (flags & MESH_FLIP_Z)
    {
        // Not implemented yet
        ASSERT(false);
    }

    result.positions = make_buffer(GL_ARRAY_BUFFER,
                                   result.num_positions * sizeof(GLfloat),
                                   result.buffer.positions, GL_STATIC_DRAW);

    if (result.num_normals)
        result.normals = make_buffer(GL_ARRAY_BUFFER,
                                     result.num_normals * sizeof(GLfloat),
                                     result.buffer.normals, GL_STATIC_DRAW);

    if (result.num_texels)
        result.texels = make_buffer(GL_ARRAY_BUFFER,
                                       result.num_texels * sizeof(GLfloat),
                                       result.buffer.texels, GL_STATIC_DRAW);

    if (result.num_indices)
        result.indices = make_buffer(GL_ELEMENT_ARRAY_BUFFER,
                                     result.num_indices * sizeof(GLuint),
                                     result.buffer.indices, GL_STATIC_DRAW);
    return result;
}

#endif // SM_ASSET_IMPLEMENTATION
