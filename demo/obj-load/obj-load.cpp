#define USE_NEW_MATH
#include "sumo.h"
#include <stdio.h>
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

#if 0
MeshAsset mitsuba;
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
    glDrawElements(GL_TRIANGLES, mitsuba.num_indices, GL_UNm_SIGNED_INT, 0);

}

#include "sumo.cpp"
#else
MeshAsset mesh;
Mesh cube;
GLuint diffusemap;
RenderPass pass;
RenderPass sdf_pass;
RenderPass march3d_pass;

#define SDF_RES 64
struct SDF
{
    float values[SDF_RES*SDF_RES*SDF_RES];
    GLuint texture;
} sdf;

// http://iquilezles.org/www/articles/distfunctions/distfunctions.htm
float dot2( vec3 v ) { return m_dot(v,v); }
float ud_triangle( vec3 p, vec3 a, vec3 b, vec3 c )
{
    vec3 ba = b - a; vec3 pa = p - a;
    vec3 cb = c - b; vec3 pb = p - b;
    vec3 ac = a - c; vec3 pc = p - c;
    vec3 nor = m_cross( ba, ac );

    return sqrt(
    (m_sign(m_dot(m_cross(ba,nor),pa)) +
     m_sign(m_dot(m_cross(cb,nor),pb)) +
     m_sign(m_dot(m_cross(ac,nor),pc))<2.0)
     ?
     m_min( m_min(
     dot2(ba*m_clamp(m_dot(ba,pa)/dot2(ba),0.0,1.0)-pa),
     dot2(cb*m_clamp(m_dot(cb,pb)/dot2(cb),0.0,1.0)-pb) ),
     dot2(ac*m_clamp(m_dot(ac,pc)/dot2(ac),0.0,1.0)-pc) )
     :
     m_dot(nor,pa)*m_dot(nor,pa)/dot2(nor) );
}

void init()
{
    mesh = load_mesh("assets/models/sdftest/sdftest.sumo_asset", MESH_NORMALIZE);
    printf("Loaded mesh (%dp/%dn/%dt/%di)\n",
           mesh.num_positions / 3,
           mesh.num_normals / 3,
           mesh.num_texels / 2,
           mesh.num_indices);

    pass = load_render_pass("demo/obj-load/diffuse.vs",
                            "demo/obj-load/diffuse.fs");
    sdf_pass = load_render_pass("demo/obj-load/sdfvis.vs",
                                "demo/obj-load/sdfvis.fs");
    march3d_pass = load_render_pass("demo/obj-load/march3d.vs",
                                    "demo/obj-load/march3d.fs");
    diffusemap = so_load_tex2d("assets/models/sdftest/diffuse.png");
    cube = make_cube();

#if 0
    u32 sdf_index = 0;
    for (u32 z = 0; z < SDF_RES; z++)
    for (u32 y = 0; y < SDF_RES; y++)
    for (u32 x = 0; x < SDF_RES; x++)
    {
        r32 d_m_min = 8.0f;
        vec3 p = vec3(-1.0f + 2.0f * x / SDF_RES,
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
            if (d < d_m_min) d_m_min = d;
        }
        sdf.values[sdf_index++] = d_m_min;
    }

    FILE *f = fopen("demo/obj-load/temp.sdf", "wb");
    fwrite((char*)sdf.values, sizeof(char), sizeof(r32)*SDF_RES*SDF_RES*SDF_RES, f);
    fclose(f);
#else
    float *sdf_data = (float*)so_read_file("demo/obj-load/temp.sdf");
    for (u32 i = 0; i < SDF_RES*SDF_RES*SDF_RES; i++)
        sdf.values[i] = sdf_data[i];
#endif

    // u32 index = 0;
    // for (u32 z = 0; z < SDF_RES; z++)
    // for (u32 y = 0; y < SDF_RES; y++)
    // for (u32 x = 0; x < SDF_RES; x++)
    // {
    //     r32 xf = -1.0f + 2.0f * x / SDF_RES;
    //     r32 yf = -1.0f + 2.0f * y / SDF_RES;
    //     r32 zf = -1.0f + 2.0f * z / SDF_RES;
    //     vec3 p = m_vec3(xf, yf, zf);
    //     r32 df1 = m_length(p) - 0.3f;
    //     r32 df2 = m_length(p - m_vec3(0.4f, 0.2f, 0.1f)) - 0.25f;
    //     sdf.values[index] = m_min(df1, df2);
    //     // if (x == 0 || x == SDF_RES - 1 ||
    //     //     y == 0 || y == SDF_RES - 1 ||
    //     //     z == 0 || z == SDF_RES - 1)
    //     //     sdf.values[index] = 1.0f;
    //     index++;
    // }

    sdf.texture = so_make_tex3d(sdf.values,
                                SDF_RES, SDF_RES, SDF_RES,
                                GL_R32F, GL_RED, GL_FLOAT,
                                GL_LINEAR, GL_LINEAR,
                                GL_CLAMP_TO_EDGE,
                                GL_CLAMP_TO_EDGE,
                                GL_CLAMP_TO_EDGE);
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
    so_draw_fullscreen_quad();

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    begin(&march3d_pass);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, sdf.texture);
    uniformi("channel", 0);
    uniformf("projection", projection);
    uniformf("view", view);
    uniformf("scale", 1.0f);
    mat4 inv_view = m_se3_inverse(view);
    vec3 camera_origin = (inv_view * m_vec4(0, 0, 0, 1)).xyz;
    vec3 camera_dir = (inv_view * m_vec4(0, 0, -1, 0)).xyz;
    uniformf("camera_dir", camera_dir);
    glBindBuffer(GL_ARRAY_BUFFER, cube.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.ibo);
    attribfv("position", 3, 6, 0);
    glDrawElements(GL_TRIANGLES, cube.index_count, cube.index_type, 0);
}

#include "sumo.cpp"
#endif
