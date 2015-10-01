#include "tiny_obj_loader.cpp"
#define SO_ASSET_IMPLEMENTATION
#include "../so_asset.h"
#include <iostream>
using namespace std;
using namespace tinyobj;

struct Buffer
{
    char   *data;
    size_t  size;
    size_t  used;
};

Buffer make_buffer(size_t nbytes)
{
    Buffer result = {};
    result.data = new char[nbytes];
    result.size = nbytes;
    result.used = 0;
    return result;
}

void free_buffer(Buffer *buffer)
{
    buffer->used = 0;
    buffer->size = 0;
    if (buffer->data)
        delete[] buffer->data;
}

void write_bytes(Buffer *destination, char *data, size_t nbytes)
{
    assert(destination->used + nbytes <= destination->size);
    for (size_t i = 0; i < nbytes; i++)
    {
        destination->data[destination->used] = data[i];
        destination->used++;
    }
}

void export_obj(char *name, // does not include .obj
                char *material_directory)
{
    vector<shape_t> shapes;
    vector<material_t> materials;
    char full_name[256];
    sprintf(full_name, "%s.obj", name);
    string result = LoadObj(shapes, materials,
                            full_name, material_directory);
    if (result != "")
    {
        cout << result << endl;
        exit(-1);
    }

    printf("Exporting %s\n", name);

    size_t npositions = 0;
    size_t nnormals = 0;
    size_t ntexels = 0;
    size_t nindices = 0;
    for (int i = 0; i < shapes.size(); i++)
    {
        npositions += shapes[i].mesh.positions.size();
        nnormals += shapes[i].mesh.normals.size();
        ntexels += shapes[i].mesh.texcoords.size();
        nindices += shapes[i].mesh.indices.size();
    }

    Buffer positions = make_buffer(npositions * sizeof(float));
    Buffer normals   = make_buffer(nnormals   * sizeof(float));
    Buffer texels    = make_buffer(ntexels    * sizeof(float));
    Buffer indices   = make_buffer(nindices   * sizeof(unsigned int));

    // TODO: endian-correct serialization

    unsigned int index_offset = 0;
    for (int i = 0; i < shapes.size(); i++)
    {
        mesh_t mesh = shapes[i].mesh;
        write_bytes(&positions, (char*)&mesh.positions[0],  sizeof(float) * mesh.positions.size());
        write_bytes(&normals,   (char*)&mesh.normals[0],    sizeof(float) * mesh.normals.size());
        write_bytes(&texels,    (char*)&mesh.texcoords[0],  sizeof(float) * mesh.texcoords.size());

        for (int j = 0; j < mesh.indices.size(); j++)
        {
            unsigned int index = mesh.indices[j] + index_offset;
            write_bytes(&indices, (char*)&index, sizeof(unsigned int));
        }
        index_offset += mesh.positions.size();
    }

    printf("%d positions\n%d normals\n%d texels\n%d indices\n\n",
           npositions / 3, nnormals / 3, ntexels / 2, nindices);

    char output_name[256];
    sprintf(output_name, "%s.sumo_asset", name);
    FILE *output = fopen(output_name, "wb");

    unsigned int header[4] = {
        npositions,
        nnormals,
        ntexels,
        nindices
    };

    fwrite((char*)&header, sizeof(char), sizeof(header), output);
    fwrite(positions.data, sizeof(char), positions.size, output);
    fwrite(normals.data,   sizeof(char), normals.size,   output);
    fwrite(texels.data,    sizeof(char), texels.size, output);
    fwrite(indices.data,   sizeof(char), indices.size,   output);
    fclose(output);

    free_buffer(&positions);
    free_buffer(&normals);
    free_buffer(&texels);
    free_buffer(&indices);

    // for (int i = 0; i < materials.size(); i++)
    //     cout << materials[i].name << endl;
}

int main(int argc, char **argv)
{
    export_obj("assets/models/mitsuba/mitsuba", "assets/models/mitsuba/");
    export_obj("assets/models/teapot/teapot", "assets/models/teapot/");

#if 1
    FILE *input = fopen("assets/models/teapot/teapot.sumo_asset", "rb");
    assert(input);
    fseek(input, 0, SEEK_END);
    long size = ftell(input);
    rewind(input);
    char *data = new char[size];
    fread(data, 1, size, input);

    float *positions;
    float *normals;
    float *texels;
    unsigned int *indices;
    unsigned int num_positions;
    unsigned int num_normals;
    unsigned int num_texels;
    unsigned int num_indices;
    so_read_mesh_from_memory(data,
        &positions, &normals, &texels, &indices,
        &num_positions, &num_normals, &num_texels, &num_indices);

    printf("%d positions\n%d normals\n%d texels\n%d indices\n\n",
           num_positions / 3, num_normals / 3, num_texels / 2, num_indices);

    for (int i = 0; i < 10; i++) printf("%.2f\n", texels[i]);
    fclose(input);

    // printf("Positions: %d\nNormals: %d\ntexels: %d\nIndices: %d\n",
    //        header->npositions, header->nnormals,
    //        header->ntexels, header->nindices);
#endif

    return 0;
}
