#ifndef SO_ASSET_HEADER_INCLUDE
#define SO_ASSET_HEADER_INCLUDE

extern void
so_read_mesh_from_memory(void          *data,
                         float        **out_positions,
                         float        **out_normals,
                         float        **out_texels,
                         unsigned int **out_indices,
                         unsigned int  *out_num_positions,
                         unsigned int  *out_num_normals,
                         unsigned int  *out_num_texels,
                         unsigned int  *out_num_indices);

#endif // SO_ASSET_HEADER_INCLUDE
#ifdef SO_ASSET_IMPLEMENTATION

char *soi_read_u32(char *data, unsigned int *result)
{
    *result = *(unsigned int*)(data);
    return data + sizeof(unsigned int);
}

char *soi_read_u32(char *data, unsigned int **result, unsigned int count)
{
    *result = (unsigned int*)(data);
    return data + sizeof(unsigned int) * count;
}

char *soi_read_r32(char *data, float **result, unsigned int count)
{
    *result = (float*)data;
    return data + sizeof(float) * count;
}

void
so_read_mesh_from_memory(void          *data,
                         float        **out_positions,
                         float        **out_normals,
                         float        **out_texels,
                         unsigned int **out_indices,
                         unsigned int  *out_num_positions,
                         unsigned int  *out_num_normals,
                         unsigned int  *out_num_texels,
                         unsigned int  *out_num_indices)
{
    char *read = (char*)data;
    read = soi_read_u32(read, out_num_positions);
    read = soi_read_u32(read, out_num_normals);
    read = soi_read_u32(read, out_num_texels);
    read = soi_read_u32(read, out_num_indices);
    read = soi_read_r32(read, out_positions, *out_num_positions);
    read = soi_read_r32(read, out_normals, *out_num_normals);
    read = soi_read_r32(read, out_texels, *out_num_texels);
    read = soi_read_u32(read, out_indices, *out_num_indices);
}

#endif // SO_ASSET_IMPLEMENTATION
