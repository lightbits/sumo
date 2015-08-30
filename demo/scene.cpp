#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_TITLE "Scene Data Manipulator"

char *SCENE_GUI_VS =
"#version 150                                                       \n"
"in vec3 position;                                                  \n"
"in vec4 color;                                                     \n"
"uniform mat4 projection;                                           \n"
"uniform mat4 view;                                                 \n"
"uniform mat4 model;                                                \n"
"out vec4 v_color;                                                  \n"
"void main()                                                        \n"
"{                                                                  \n"
"    v_color = color;                                               \n"
"    gl_Position = projection * view * model * vec4(position, 1.0); \n"
"}                                                                  \n";

char *SCENE_GUI_FS =
"#version 150           \n"
"in vec4 v_color;       \n"
"out vec4 f_color;      \n"
"void main()            \n"
"{                      \n"
"    f_color = v_color; \n"
"}                      \n";

char *SHADER_VS =
"#version 150                                              \n"
"in vec3 position;                                         \n"
"in vec3 normal;                                           \n"
"uniform mat4 projection;                                  \n"
"uniform mat4 view;                                        \n"
"uniform mat4 model;                                       \n"
"out vec3 v_world;                                         \n"
"out vec3 v_normal;                                        \n"
"void main()                                               \n"
"{                                                         \n"
"    v_world = (model * vec4(position, 1.0)).xyz;          \n"
"    v_normal = (model * vec4(normal, 0.0)).xyz;           \n"
"    gl_Position = projection * view * vec4(v_world, 1.0); \n"
"}                                                         \n";

char *SHADER_FS =
"#version 150                                       \n"
"in vec3 v_world;                                   \n"
"in vec3 v_normal;                                  \n"
"out vec4 f_color;                                  \n"
"vec3 diffuse(vec3 c, vec3 l, vec3 n)               \n"
"{                                                  \n"
"    return max(dot(n, l), 0.0) * c;                \n"
"}                                                  \n"
"void main()                                        \n"
"{                                                  \n"
"    vec3 n = normalize(v_normal);                  \n"
"    vec3 color = vec3(0.0);                        \n"
"    vec3 c1 = vec3(1.0, 0.98, 0.92);               \n"
"    vec3 l1 = normalize(vec3(0.8, 1.0, 0.5));      \n"
"    vec3 c2 = vec3(0.12, 0.15, 0.25);              \n"
"    vec3 l2 = normalize(vec3(-1.0, -0.3, -0.5));   \n"
"    color += diffuse(c1, l1, n);                   \n"
"    color += diffuse(c2, l2, n);                   \n"
"    f_color = vec4(color, 1.0);                    \n"
"}                                                  \n";

RenderPass scene_gui_pass;

void pick_vector_perspective(vec2 ndc, r32 zn, mat4 projection,
                             mat4 view, vec3 *dir, vec3 *pos)
{
    r32 xc = ndc.x * zn;
    r32 yc = ndc.y * zn;
    r32 xv = xc / projection.x.x;
    r32 yv = yc / projection.y.y;
    r32 zv = -zn;

    // the camera frame expressed in world frame
    vec3 camera_x = vec3(view.x.x, view.y.x, view.z.x);
    vec3 camera_y = vec3(view.x.y, view.y.y, view.z.y);
    vec3 camera_z = vec3(view.x.z, view.y.z, view.z.z);

    *dir = camera_x * xv + camera_y * yv + camera_z * zv;
    *dir = normalize(*dir);
    *pos = camera_x * (-view.w.x) + camera_y * (-view.w.y) + camera_z * (-view.w.z);
}

// ro: ray origin
// rd: ray direction
//  n: plane normal
//  d: normal distance from origin to plane
// *t: distance along rd to intersection point
bool intersect_ray_plane(vec3 ro, vec3 rd, vec3 n, r32 d, r32 *t)
{
    r32 a = dot(rd, n);
    if (a != 0.0f)
    {
        *t = (d * dot(n, n) - dot(ro, n)) / a;
        return true;
    }
    return false;
}

// ro: ray origin
// rd: ray direction
// sc: sphere centre
//  r: sphere radius
// *t: distance along rd to first intersection point
bool intersect_ray_sphere(vec3 ro, vec3 rd, vec3 centre, r32 radius, r32 *t)
{
    r32 a = dot(rd, rd);
    r32 b = 2.0f*dot(ro - centre, rd);
    r32 c = dot(ro - centre, ro - centre) - radius*radius;
    r32 d = b*b - 4*a*c;
    if (d >= 0.0f)
    {
        r32 t1 = (-b + sqrt(d)) / (2.0f * a);
        r32 t2 = (-b - sqrt(d)) / (2.0f * a);
        if (t1 >= 0.0f && t2 >= 0.0f)
        {
            *t = min(t1, t2);
            return true;
        }
        else if (t1 >= 0.0f)
        {
            *t = t1;
            return true;
        }
        else if (t2 >= 0.0f)
        {
            *t = t2;
            return true;
        }
    }
    return false;
}

struct SceneObject
{
    vec3 position;
    struct Rotation
    {
        vec3 euler;
        vec4 quaternion;
        mat4 matrix;
    } rotation;
    struct Bounds
    {
        vec3 offset; // from position
        r32 radius;
    } bounds;
};

#define MaxSceneObjects 256
struct Scene
{
    SceneObject objects[MaxSceneObjects];
    u32 object_count;
};

Scene make_scene()
{
    Scene scene = {};
    return scene;
}

u32 scene_add_object(Scene *scene, vec3 bounds_offset, r32 bounds_radius)
{
    ASSERT(scene->object_count < MaxSceneObjects);
    u32 index = scene->object_count;
    SceneObject *object = scene->objects + index;
    object->position = vec3(0, 0, 0);
    object->rotation.euler = vec3(0, 0, 0);
    object->rotation.quaternion = vec4(0, 0, 0, 0);
    object->rotation.matrix = mat_identity();
    object->bounds.offset = bounds_offset;
    object->bounds.radius = bounds_radius;
    scene->object_count++;
    return index;
}

void scene_tick(Scene *scene, Input io, mat4 view,
                r32 zn, mat4 projection,
                s32 *selected_object_index)
{
    vec3 ro, rd;
    vec2 ndc = (io.mouse.pos / vec2(WINDOW_WIDTH, -WINDOW_HEIGHT)) * 2.0f + vec2(-1.0f, +1.0f);
    pick_vector_perspective(ndc, zn, projection, view, &rd, &ro);
    r32 closest_t = -1.0f;
    s32 closest_i = -1;
    for (u32 i = 0; i < scene->object_count; i++)
    {
        SceneObject *obj = scene->objects + i;
        vec3 c = obj->position + obj->bounds.offset;
        r32 r = obj->bounds.radius;
        r32 t;
        if (intersect_ray_sphere(ro, rd, c, r, &t))
        {
            if (t < closest_t || closest_i < 0)
            {
                closest_t = t;
                closest_i = i;
            }
        }
    }
    *selected_object_index = closest_i;
}

GLuint axes;
GLuint grid;
Scene scene;

void scene_present_gui(mat4 projection, mat4 view)
{
    begin(&scene_gui_pass);
    glBindBuffer(GL_ARRAY_BUFFER, grid);
    attribfv("position", 3, 7, 0);
    attribfv("color", 4, 7, 3);
    uniformf("projection", projection);
    uniformf("view", view);
    uniformf("model", mat_scale(1.0f));
    glDrawArrays(GL_LINES, 0, 16*4);
}

RenderPass pass;
#define NumCubes 4
Mesh cube;

GLuint scene_make_axes()
{
    r32 v[] = {
        0.0f, 0.0f, 0.0f, 0.95f, 0.33f, 0.25f, 1.0f,
        1.0f, 0.0f, 0.0f, 0.95f, 0.33f, 0.25f, 1.0f,

        0.0f, 0.0f, 0.0f, 0.25f, 0.95f, 0.33f, 1.0f,
        0.0f, 1.0f, 0.0f, 0.25f, 0.95f, 0.33f, 1.0f,

        0.0f, 0.0f, 0.0f, 0.25f, 0.33f, 0.95f, 1.0f,
        0.0f, 0.0f, 1.0f, 0.25f, 0.33f, 0.95f, 1.0f
    };
    return make_buffer(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
}

GLuint scene_make_grid()
{
    const u32 GridLinesDim = 16;
    struct Vertex
    {
        vec3 position;
        vec4 color;
    };
    Vertex v[GridLinesDim*4];
    u32 ix = 0;
    u32 iz = GridLinesDim*2;
    for (u32 i = 0; i < GridLinesDim; i++)
    {
        r32 t = -1.0f + 2.0f * i / (r32)(GridLinesDim - 1);

        vec4 x_color = vec4(1.0f, 1.0f, 1.0f, 0.2f);
        vec4 z_color = vec4(1.0f, 1.0f, 1.0f, 0.2f);
        if (i == GridLinesDim / 2)
        {
            x_color = vec4(1.0f, 0.33f, 0.2f, 0.5f);
            z_color = vec4(0.2f, 0.33f, 1.0f, 0.5f);
        }

        v[ix].position = vec3(-1.0f, 0.0f, t);
        v[ix++].color = x_color;

        v[ix].position = vec3(+1.0f, 0.0f, t);
        v[ix++].color = x_color;

        v[iz].position = vec3(t, 0.0f, -1.0f);
        v[iz++].color = z_color;

        v[iz].position = vec3(t, 0.0f, 1.0f);
        v[iz++].color = z_color;
    }
    return make_buffer(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
}

void init()
{
    RenderPassSource source = {
        SHADER_VS,
        SHADER_FS
    };
    source.from_memory = true;
    pass = make_render_pass(source);

    RenderPassSource scene_gui_pass_source = {
        SCENE_GUI_VS,
        SCENE_GUI_FS
    };
    scene_gui_pass_source.from_memory = true;
    scene_gui_pass = make_render_pass(scene_gui_pass_source);
    cube = make_cube();

    scene = make_scene();
    for (u32 i = 0; i < NumCubes; i++)
        scene_add_object(&scene, vec3(0, 0, 0), 0.1f);

    axes = scene_make_axes();
    grid = scene_make_grid();
}

void tick(Input io, r32 t, r32 dt)
{
    persist s32 selected_object_index = -1;
    mat4 projection = mat_perspective(PI / 4.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.1f, 10.0f);
    mat4 view = mat_translate(0.0f, 0.0f, -1.0f) * mat_rotate_x(0.3f) * mat_rotate_y(0.2f);

    #if 0
    vec2 ndc = (io.mouse.pos / vec2(WINDOW_WIDTH, -WINDOW_HEIGHT)) * 2.0f + vec2(-1.0f, +1.0f);
    vec3 dir, pos;
    pick_vector_perspective(ndc, 0.1f, projection, view, &dir, &pos);
    r32 t_plane;
    intersect_ray_plane(pos, dir, vec3(0, 1, 0), 0.0f, &t_plane);
    vec3 traced = pos + dir * t_plane;
    mat4 model = mat_translate(traced) * mat_scale(0.1f);
    #endif

    vec4 clear_color = vec4(0.01f, 0.05f, 0.1f, 1.0f);

    s32 hover = -1;
    scene_tick(&scene, io, view, 0.1f, projection, &hover);
    if (hover >= 0 && io.mouse.left.released)
    {
        selected_object_index = hover;
    }
    else if (hover < 0 && io.mouse.left.released)
    {
        selected_object_index = -1;
    }

    if (selected_object_index >= 0)
    {
        SceneObject *obj = scene.objects + selected_object_index;
        if (io.key.down['a'])
            obj->position.x -= 0.2f * dt;
        else if (io.key.down['d'])
            obj->position.x += 0.2f * dt;
        if (io.key.down['w'])
            obj->position.z -= 0.2f * dt;
        else if (io.key.down['s'])
            obj->position.z += 0.2f * dt;
    }

    // mat4 model = mat_translate(selected->position) *
    //              mat_rotate_z(selected->rotation.euler.z) *
    //              mat_rotate_y(selected->rotation.euler.y) *
    //              mat_rotate_x(selected->rotation.euler.x) *
    //              mat_scale(selected->scale);

    depth_test(true, GL_LEQUAL);
    depth_write(true);
    blend_mode(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    clear(clear_color, 1.0f);
    begin(&pass);
    glBindBuffer(GL_ARRAY_BUFFER, cube.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.ibo);
    attribfv("position", 3, 6, 0);
    attribfv("normal", 3, 6, 3);
    uniformf("projection", projection);
    uniformf("view", view);

    for (u32 i = 0; i < scene.object_count; i++)
    {
        mat4 model = mat_translate(scene.objects[i].position) * mat_scale(0.1f);
        uniformf("model", model);
        glDrawElements(GL_TRIANGLES, cube.index_count, cube.index_type, 0);
    }

    scene_present_gui(projection, view);

    ImGui::NewFrame();
    ImGui::Begin("Diffuse Shader");
    char text[255];
    sprintf(text, "%d", hover);
    ImGui::Text(text);
    ImGui::End();
    ImGui::Render();
}

#include "sumo.cpp"
