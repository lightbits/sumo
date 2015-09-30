#include "tiny_obj_loader.cpp"
#include <iostream>
using namespace std;
using namespace tinyobj;

struct Buffer
{
    char   *data;
    size_t  size;
    size_t  used;
};

struct MeshAssetHeader
{
    int npositions;
    int ntexcoords;
    int nnormals;
    int nindices;
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
    size_t ntexcoords = 0;
    size_t nindices = 0;
    for (int i = 0; i < shapes.size(); i++)
    {
        npositions += shapes[i].mesh.positions.size();
        nnormals += shapes[i].mesh.normals.size();
        ntexcoords += shapes[i].mesh.texcoords.size();
        nindices += shapes[i].mesh.indices.size();
    }

    Buffer positions = make_buffer(npositions * sizeof(float));
    Buffer normals   = make_buffer(nnormals *   sizeof(float));
    Buffer texcoords = make_buffer(ntexcoords * sizeof(float));
    Buffer indices   = make_buffer(nindices *   sizeof(unsigned int));

    unsigned int index_offset = 0;
    for (int i = 0; i < shapes.size(); i++)
    {
        mesh_t mesh = shapes[i].mesh;
        // TODO: Endianness should be included in the asset file
        write_bytes(&positions, (char*)&mesh.positions[0],  sizeof(float) * mesh.positions.size());
        write_bytes(&normals,   (char*)&mesh.normals[0],    sizeof(float) * mesh.normals.size());
        write_bytes(&texcoords, (char*)&mesh.texcoords[0],  sizeof(float) * mesh.texcoords.size());

        for (int j = 0; j < mesh.indices.size(); j++)
        {
            unsigned int index = mesh.indices[j] + index_offset;
            write_bytes(&indices, (char*)&index, sizeof(unsigned int));
        }
        index_offset += mesh.positions.size();
    }

    printf("%d positions\n%d normals\n%d texcoords\n%d indices\n\n",
           npositions / 3, nnormals / 3, ntexcoords / 2, nindices);

    char output_name[256];
    sprintf(output_name, "%s.sumo_asset", name);
    FILE *output = fopen(output_name, "wb");

    // TODO: Figure out cleaner and less bug-prone way to
    // include header information into asset file. Declspec
    // might be necessary.
    int header[] = {
        npositions,
        nnormals,
        ntexcoords,
        nindices
     };

    fwrite((void*)header,  sizeof(char), sizeof(header), output);
    fwrite(positions.data, sizeof(char), positions.size, output);
    fwrite(normals.data,   sizeof(char), normals.size,   output);
    fwrite(texcoords.data, sizeof(char), texcoords.size, output);
    fwrite(indices.data,   sizeof(char), indices.size,   output);
    fclose(output);

    free_buffer(&positions);
    free_buffer(&normals);
    free_buffer(&texcoords);
    free_buffer(&indices);

    // for (int i = 0; i < materials.size(); i++)
    //     cout << materials[i].name << endl;
}

int main(int argc, char **argv)
{
    export_obj("assets/models/mitsuba", "assets/models/");
    export_obj("assets/models/wt_teapot", "assets/models/");
    export_obj("assets/models/cornell_box", "assets/models/");

#if 0
    FILE *input = fopen("assets/models/cornell_box.sumo_asset", "rb");
    fseek(input, 0, SEEK_END);
    long size = ftell(input);
    rewind(input);
    char *data = new char[size];
    fread(data, 1, size, input);

    // Yuck!
    int *header = (int*)data;
    int npositions = header[0];
    int nnormals = header[1];
    int ntexcoords = header[2];
    int nindices = header[3];
    float *positions      = (float*)(data + sizeof(int) * 4);
    float *normals        = (float*)(data + sizeof(int) * 4 + npositions * sizeof(float));
    float *texcoords      = (float*)(data + sizeof(int) * 4 + npositions * sizeof(float) + nnormals * sizeof(float));
    unsigned int *indices = (unsigned int*)(data + sizeof(int) * 4 + npositions * sizeof(float) + nnormals * sizeof(float) + ntexcoords * sizeof(float));
    fclose(input);

    // printf("Positions: %d\nNormals: %d\nTexcoords: %d\nIndices: %d\n",
    //        header->npositions, header->nnormals,
    //        header->ntexcoords, header->nindices);
#endif

    return 0;
}
