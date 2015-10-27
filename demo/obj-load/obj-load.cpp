#include "sumo.h"
#define SO_ASSET_IMPLEMENTATION
#include "so_asset.h"
#include <stdio.h>
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

// TODO: Add to shared library
char*
sm_read_file(char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        printf("Failed to open file %s\n", filename);
        exit(-1);
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);
    char *data = new char[size];
    fread(data, 1, size, file);
    fclose(file);
    return data;
}

// TODO: Add to so_texture
GLuint
so_make_tex3d(void *data,
              int width, int height, int depth,
              GLenum internal_format,
              GLenum data_format,
              GLenum data_type,
              GLenum min_filter,
              GLenum mag_filter,
              GLenum wrap_s,
              GLenum wrap_t,
              GLenum wrap_r)
{
    GLuint result = 0;
    glGenTextures(1, &result);
    glBindTexture(GL_TEXTURE_3D, result);
    glTexImage3D(GL_TEXTURE_3D, 0, internal_format,
                 width, height, depth, 0,
                 data_format, data_type, data);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, wrap_s);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, wrap_t);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, wrap_r);
    glBindTexture(GL_TEXTURE_3D, 0);

    // TODO: Undef when added to so_texture
    #if 0
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        const char *reason = soi_gl_error_message(error);
        printf("Failed to make 3D texture: %s\n", reason);
        result = 0;
    }
    #endif
    return result;
}

struct so_LoadedMesh
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

so_LoadedMesh load_mesh(char *name)
{
    // TODO: File not found
    char full_name[256];
    sprintf(full_name, "assets/models/%s/%s.sumo_asset", name, name);
    char *data = sm_read_file(full_name);

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

    // if normalize
    float max_position = 0.0f;
    for (u32 i = 0; i < result.num_positions; i++)
    {
        float a = abs(result.buffer.positions[i]);
        if (a > max_position)
            max_position = a;
    }

    for (u32 i = 0; i < result.num_positions; i++)
    {
        result.buffer.positions[i] /= max_position;
    }

    printf("Loaded mesh %s (%dp/%dn/%dt/%di)\n",
           name,
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
    return result;
}

#if 0
so_LoadedMesh mitsuba;
GLuint diffusemap;
RenderPass pass;

void init()
{
    mitsuba = load_mesh("mitsuba");
    pass = load_render_pass("demo/obj-load/default.vs",
                            "demo/obj-load/default.fs");
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
#else
so_LoadedMesh mesh;
GLuint diffusemap;
RenderPass pass;
RenderPass sdf_pass;

#define SDF_RES 64
struct SDF
{
    float values[SDF_RES*SDF_RES*SDF_RES];
    GLuint texture;
} sdf;

// http://iquilezles.org/www/articles/distfunctions/distfunctions.htm
float dot2( vec3 v ) { return dot(v,v); }
float ud_triangle( vec3 p, vec3 a, vec3 b, vec3 c )
{
    vec3 ba = b - a; vec3 pa = p - a;
    vec3 cb = c - b; vec3 pb = p - b;
    vec3 ac = a - c; vec3 pc = p - c;
    vec3 nor = cross( ba, ac );

    return sqrt(
    (sign(dot(cross(ba,nor),pa)) +
     sign(dot(cross(cb,nor),pb)) +
     sign(dot(cross(ac,nor),pc))<2.0)
     ?
     min( min(
     dot2(ba*clampf(dot(ba,pa)/dot2(ba),0.0,1.0)-pa),
     dot2(cb*clampf(dot(cb,pb)/dot2(cb),0.0,1.0)-pb) ),
     dot2(ac*clampf(dot(ac,pc)/dot2(ac),0.0,1.0)-pc) )
     :
     dot(nor,pa)*dot(nor,pa)/dot2(nor) );
}

void init()
{
    mesh = load_mesh("sdftest");
    pass = load_render_pass("demo/obj-load/diffuse.vs",
                            "demo/obj-load/diffuse.fs");
    sdf_pass = load_render_pass("demo/obj-load/sdfvis.vs",
                                "demo/obj-load/sdfvis.fs");
    diffusemap = so_load_tex2d("assets/models/sdftest/diffuse.png");

#if 0
    u32 sdf_index = 0;
    for (u32 z = 0; z < SDF_RES; z++)
    for (u32 y = 0; y < SDF_RES; y++)
    for (u32 x = 0; x < SDF_RES; x++)
    {
        r32 d_min = 8.0f;
        vec3 p = V3(-1.0f + 2.0f * x / SDF_RES,
                      -1.0f + 2.0f * y / SDF_RES,
                      -1.0f + 2.0f * z / SDF_RES);
        for (u32 f = 0; f < mesh.num_indices; f += 3)
        {
            // indices of triangle points
            u32 i0 = mesh.buffer.indices[f];
            u32 i1 = mesh.buffer.indices[f+1];
            u32 i2 = mesh.buffer.indices[f+2];

            // points of triangle
            vec3 a = *(((vec3*)mesh.buffer.positions) + i0);
            vec3 b = *(((vec3*)mesh.buffer.positions) + i1);
            vec3 c = *(((vec3*)mesh.buffer.positions) + i2);

            r32 d = ud_triangle(p, a, b, c);
            if (d < d_min) d_min = d;
        }
        sdf.values[sdf_index++] = d_min;
    }

    FILE *f = fopen("demo/obj-load/temp.sdf", "wb");
    fwrite((char*)sdf.values, sizeof(char), sizeof(r32)*SDF_RES*SDF_RES*SDF_RES, f);
    fclose(f);
#else
    float *sdf_data = (float*)sm_read_file("demo/obj-load/temp.sdf");
    for (u32 i = 0; i < SDF_RES*SDF_RES*SDF_RES; i++)
        sdf.values[i] = sdf_data[i];
#endif

    sdf.texture = so_make_tex3d(sdf.values,
                                SDF_RES, SDF_RES, SDF_RES,
                                GL_R32F, GL_RED, GL_FLOAT,
                                GL_LINEAR, GL_LINEAR,
                                GL_CLAMP_TO_EDGE,
                                GL_CLAMP_TO_EDGE,
                                GL_CLAMP_TO_EDGE);
}

void sm_draw_quad()
{
    persist bool loaded = false;
    persist GLuint buffer = 0;
    if (!loaded)
    {
        float data[] = {
            -1.0f, -1.0f,
            +1.0f, -1.0f,
            +1.0f, +1.0f,
            +1.0f, +1.0f,
            -1.0f, +1.0f,
            -1.0f, -1.0f
        };
        buffer = make_buffer(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
        loaded = 1;
    }
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    attribfv("position", 2, 2, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void tick(Input io, float t, float dt)
{
    persist r32 rotate_y = 2.7f;
    persist r32 drotate_y = 0.0f;
    persist r32 dzoom = 0.0f;
    persist r32 rzoom = 1.0f;
    persist r32 zoom = 1.0f;

    if (io.mouse.left.down)
        drotate_y += io.mouse.rel.x * dt;
    rotate_y += drotate_y * dt;
    drotate_y *= 0.95f;

    if (io.key.released['z'])
    {
        if (rzoom > 1.0f)
            rzoom = 1.0f;
        else
            rzoom = 2.0f;
    }
    r32 ddzoom = 88.0f * (rzoom - zoom) - 13.8f * dzoom;
    dzoom += ddzoom * dt;
    zoom += dzoom * dt;

    mat4 model = mat_scale(1.0f);
    mat4 view = mat_translate(0.0f, -0.2f, -4.0f) *
                mat_rotate_x(0.4f) *
                mat_rotate_y(rotate_y);
    float aspect = (float)WINDOW_WIDTH / WINDOW_HEIGHT;
    mat4 projection = mat_ortho_depth(-aspect/zoom, +aspect/zoom, -1.0f/zoom, +1.0f/zoom, 0.0f, 10.0f);

    depth_test(true, GL_LEQUAL);
    depth_write(true);
    blend_mode(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    clear(0.0f, 0.0f, 0.0f, 1.0f, 1.0f);
    begin(&pass);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffusemap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, sdf.texture);
    uniformi("channel0", 0);
    uniformi("channel1", 1);
    uniformf("projection", projection);
    uniformf("view", view);
    uniformf("model", model);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.positions);
    attribfv("position", 3, 3, 0);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.normals);
    attribfv("normal", 3, 3, 0);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.texels);
    attribfv("texel", 2, 2, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indices);
    glDrawElements(GL_TRIANGLES, mesh.num_indices, GL_UNSIGNED_INT, 0);

    persist r32 depth = -1.0f;
    begin(&sdf_pass);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, sdf.texture);
    uniformi("channel", 0);
    uniformf("depth", 0.0f + 0.3f * sin(t));
    uniformf("projection", projection);
    uniformf("view", view);
    sm_draw_quad();
}

#include "sumo.cpp"
#endif
