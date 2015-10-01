#include "sumo.h"
#define SO_ASSET_IMPLEMENTATION
#include "so_asset.h"
#include <stdio.h>
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

struct so_LoadedMesh
{
    GLuint positions;
    GLuint normals;
    GLuint texels;
    GLuint indices;

    struct Buffer
    {
        u08 *data;
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

so_LoadedMesh load_mesh(char *name)
{
    // TODO: File not found
    char full_name[256];
    sprintf(full_name, "assets/models/%s/%s.sumo_asset", name, name);
    FILE *input = fopen(full_name, "rb");
    if (!input)
    {
        printf("Failed to load mesh: %s does not exist.\n", full_name);
        exit(-1);
    }
    fseek(input, 0, SEEK_END);
    long size = ftell(input);
    rewind(input);
    u08 *data = new u08[size];
    fread(data, 1, size, input);
    fclose(input);

    so_LoadedMesh result = {};
    result.buffer.data = data;
    so_read_mesh_from_memory(data,
        &result.buffer.positions,
        &result.buffer.normals,
        &result.buffer.texels,
        &result.buffer.indices,
        &result.num_positions,
        &result.num_normals,
        &result.num_texels,
        &result.num_indices);

    printf("Positions: %d\nNormals: %d\ntexels: %d\nIndices: %d\n",
           result.num_positions / 3,
           result.num_normals / 3,
           result.num_texels / 2,
           result.num_indices);

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

    delete[] data;
    return result;
}

so_LoadedMesh mitsuba;
RenderPass pass;

void init()
{
    mitsuba = load_mesh("mitsuba");
    pass = load_render_pass("assets/shaders/default.vs",
                            "assets/shaders/default.fs");
}

void tick(Input io, float t, float dt)
{
    mat4 model = mat_scale(1.0f);
    mat4 view = mat_translate(0.0f, -1.0f, -4.0f) *
                mat_rotate_x(0.3f) *
                mat_rotate_y(0.4f);
    // mat4 view = camera_holdclick(io, dt);
    mat4 projection = mat_perspective(PI / 4.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.1f, 10.0f);

    depth_test(true, GL_LEQUAL);
    depth_write(true);
    clear(0.95f, 0.97f, 1.0f, 1.0f, 1.0f);
    begin(&pass);
    uniformf("projection", projection);
    uniformf("view", view);
    uniformf("model", model);
    glBindBuffer(GL_ARRAY_BUFFER, mitsuba.positions);
    attribfv("position", 3, 3, 0);
    if (mitsuba.normals)
    {
        glBindBuffer(GL_ARRAY_BUFFER, mitsuba.normals);
        attribfv("normal", 3, 3, 0);
    }
    if (mitsuba.texels)
    {
        glBindBuffer(GL_ARRAY_BUFFER, mitsuba.texels);
        attribfv("texcoord", 2, 2, 0);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mitsuba.indices);
    glDrawElements(GL_TRIANGLES, mitsuba.num_indices, GL_UNSIGNED_INT, 0);

}

#include "sumo.cpp"
