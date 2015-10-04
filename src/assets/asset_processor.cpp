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

    size_t num_positions = 0;
    size_t num_normals = 0;
    size_t num_texels = 0;
    size_t num_indices = 0;
    size_t num_material_ids = 0;
    for (int i = 0; i < shapes.size(); i++)
    {
        num_positions += shapes[i].mesh.positions.size();
        num_normals += shapes[i].mesh.normals.size();
        num_texels += shapes[i].mesh.texcoords.size();
        num_indices += shapes[i].mesh.indices.size();
        num_material_ids += shapes[i].mesh.material_ids.size();
    }

    Buffer positions    = make_buffer(num_positions    * sizeof(float));
    Buffer normals      = make_buffer(num_normals      * sizeof(float));
    Buffer texels       = make_buffer(num_texels       * sizeof(float));
    Buffer indices      = make_buffer(num_indices      * sizeof(unsigned int));
    Buffer material_ids = make_buffer(num_material_ids * sizeof(int));

    // TODO: endian-correct serialization

    unsigned int index_offset = 0;
    for (int i = 0; i < shapes.size(); i++)
    {
        mesh_t mesh = shapes[i].mesh;
        write_bytes(&positions,    (char*)&mesh.positions[0],    sizeof(float) * mesh.positions.size());
        write_bytes(&normals,      (char*)&mesh.normals[0],      sizeof(float) * mesh.normals.size());
        write_bytes(&texels,       (char*)&mesh.texcoords[0],    sizeof(float) * mesh.texcoords.size());
        write_bytes(&material_ids, (char*)&mesh.material_ids[0], sizeof(int)   * mesh.material_ids.size());

        for (int j = 0; j < mesh.indices.size(); j++)
        {
            unsigned int index = mesh.indices[j] + index_offset;
            write_bytes(&indices, (char*)&index, sizeof(unsigned int));
        }
        index_offset += mesh.positions.size();
    }

    printf("%d positions\n%d normals\n%d texels\n%d indices\n%d matids\n\n",
           num_positions / 3,
           num_normals / 3,
           num_texels / 2,
           num_indices,
           num_material_ids);

    char output_name[256];
    sprintf(output_name, "%s.sumo_asset", name);
    FILE *output = fopen(output_name, "wb");

    unsigned int header[4] = {
        num_positions,
        num_normals,
        num_texels,
        num_indices
    };

    fwrite((char*)&header,    sizeof(char), sizeof(header),    output);
    fwrite(positions.data,    sizeof(char), positions.size,    output);
    fwrite(normals.data,      sizeof(char), normals.size,      output);
    fwrite(texels.data,       sizeof(char), texels.size,       output);
    fwrite(indices.data,      sizeof(char), indices.size,      output);
    // fwrite(material_ids.data, sizeof(char), material_ids.size, output);
    fclose(output);

    free_buffer(&positions);
    free_buffer(&normals);
    free_buffer(&texels);
    free_buffer(&indices);
    free_buffer(&material_ids);

    for (int i = 0; i < materials.size(); i++)
    {
        cout << materials[i].name << endl;
        cout << materials[i].ambient[0] << " " <<
                materials[i].ambient[1] << " " <<
                materials[i].ambient[2] << endl;

        cout << materials[i].diffuse[0] << " " <<
                materials[i].diffuse[1] << " " <<
                materials[i].diffuse[2] << endl;

        cout << materials[i].specular[0] << " " <<
                materials[i].specular[1] << " " <<
                materials[i].specular[2] << endl;
    }
}

int main(int argc, char **argv)
{
    // export_obj("assets/models/mitsuba/mitsuba", "assets/models/mitsuba/");
    // export_obj("assets/3rdparty/noodles/noodles", "assets/3rdparty/noodles/");
    export_obj("assets/models/teapot/teapot", "assets/models/teapot/");

    return 0;
}
