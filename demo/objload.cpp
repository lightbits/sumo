#include "sumo.h"
#include <stdio.h>
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

struct LoadedMesh
{
    GLuint positions;
    GLuint normals;
    GLuint texcoords;
    GLuint indices;
    u32 num_indices;
    u32 num_positions;
    u32 num_normals;
    u32 num_texcoords;
};

LoadedMesh load_mesh(char *name)
{
    // TODO: File not found
    char full_name[256];
    sprintf(full_name, "assets/models/%s.sumo_asset", name);
    FILE *input = fopen(full_name, "rb");
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
    GLfloat *positions = (GLfloat*)(data + sizeof(int) * 4);
    GLfloat *normals   = (GLfloat*)(data + sizeof(int) * 4 + npositions * sizeof(GLfloat));
    GLfloat *texcoords = (GLfloat*)(data + sizeof(int) * 4 + npositions * sizeof(GLfloat) + nnormals * sizeof(GLfloat));
    GLuint  *indices   = (GLuint*) (data + sizeof(int) * 4 + npositions * sizeof(GLfloat) + nnormals * sizeof(GLfloat) + ntexcoords * sizeof(GLfloat));
    fclose(input);

    printf("Positions: %d\nNormals: %d\nTexcoords: %d\nIndices: %d\n",
           npositions / 3, nnormals / 3, ntexcoords / 2, nindices);

    // int i = 0;
    // while (i < npositions)
    // {
    //     printf("%.2f ", positions[i++]);
    //     printf("%.2f ", positions[i++]);
    //     printf("%.2f\n", positions[i++]);
    // }

    // i = 0;
    // while (i < nindices)
    // {
    //     printf("%d ", indices[i++]);
    //     printf("%d ", indices[i++]);
    //     printf("%d ", indices[i++]);
    //     printf("%d ", indices[i++]);
    //     printf("%d ", indices[i++]);
    //     printf("%d\n", indices[i++]);
    // }

    LoadedMesh result = {};
    result.num_positions = npositions;
    result.num_normals = nnormals;
    result.num_texcoords = ntexcoords;
    result.num_indices = nindices;
    result.positions = make_buffer(GL_ARRAY_BUFFER,
                                   result.num_positions * sizeof(GLfloat),
                                   positions, GL_STATIC_DRAW);

    if (result.num_normals)
        result.normals = make_buffer(GL_ARRAY_BUFFER,
                                     result.num_normals * sizeof(GLfloat),
                                     normals, GL_STATIC_DRAW);

    if (result.num_texcoords)
        result.texcoords = make_buffer(GL_ARRAY_BUFFER,
                                       result.num_texcoords * sizeof(GLfloat),
                                       texcoords, GL_STATIC_DRAW);

    if (result.num_indices)
        result.indices = make_buffer(GL_ELEMENT_ARRAY_BUFFER,
                                     result.num_indices * sizeof(GLuint),
                                     indices, GL_STATIC_DRAW);
    return result;
}

LoadedMesh mitsuba;
RenderPass pass;

void init()
{
    mitsuba = load_mesh("wt_teapot");
    pass = load_render_pass("assets/shaders/default.vs",
                            "assets/shaders/default.fs");
}

void tick(Input io, float t, float dt)
{
    mat4 model = mat_scale(1.0f);
    // mat4 view = mat_translate(0.0f, 0.0f, -4.0f) *
    //             mat_rotate_x(0.3f) *
    //             mat_rotate_y(0.4f);
    mat4 view = camera_holdclick(io, dt);
    mat4 projection = mat_perspective(PI / 4.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.1f, 10.0f);

    depth_test(true, GL_LEQUAL);
    depth_write(true);
    clear(0.0f, 0.1f, 0.2f, 1.0f, 1.0f);
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
    if (mitsuba.texcoords)
    {
        glBindBuffer(GL_ARRAY_BUFFER, mitsuba.texcoords);
        attribfv("texcoord", 2, 2, 0);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mitsuba.indices);
    glDrawElements(GL_TRIANGLES, mitsuba.num_indices, GL_UNSIGNED_INT, 0);

}

#include "sumo.cpp"
