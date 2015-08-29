#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_TITLE "Scene Data Manipulator"

// struct SceneObject
// {
//     vec3 position;
//     struct Rotation
//     {
//         vec3 euler;
//         vec4 quaternion;
//         mat4 matrix;
//     } rotation;
//     struct BoundingBox
//     {
//         vec3 offset; // from position
//         vec3 size;
//     } bounds;
// };

// #define MaxSceneObjects 256
// struct Scene
// {
//     SceneObject objects[MaxSceneObjects];
//     u32 object_count;
// };

// Scene make_scene()
// {
//     Scene scene = {};
//     return scene;
// }

// u32 scene_add_object(Scene *scene, vec3 bounds_size, vec3 bounds_offset)
// {
//     ASSERT(scene->object_count < MaxSceneObjects);
//     u32 index = scene->object_count;
//     SceneObject *object = scene->objects + index;
//     object->position = vec3(0, 0, 0);
//     object->rotation.euler = vec3(0, 0, 0);
//     object->rotation.quaternion = vec4(0, 0, 0, 0);
//     object->rotation.matrix = mat_identity();
//     object->bounds.offset = bounds_offset;
//     object->bounds.size = bounds_size;
//     scene->object_count++;
//     return index;
// }

// s32 scene_trace(Scene *scene, float u, float v, mat4 view, mat4 projection)
// {
//     float u = mouse_pos.x / WINDOW_WIDTH
//     return -1;
// }

// s32 scene_tick(Scene *scene, Input io, mat4 view, mat4 projection)
// {
//     s32 selected_object_index = scene_trace(scene, io.mouse.pos, view, projection);
//     return selected_object_index;
// }

RenderPass pass;
Mesh cube;

void init()
{
    RenderPassSource source = {
        "./shaders/cube.vs",
        "./shaders/cube.fs"
    };
    pass = make_render_pass(source);
    cube = make_cube();
}

void pick_vector_perspective(vec2 ndc, float zn, mat4 projection, mat4 view,
                             vec3 *dir, vec3 *pos)
{
    float xc = ndc.x * zn;
    float yc = ndc.y * zn;
    float xv = xc / projection.x.x;
    float yv = yc / projection.y.y;
    float zv = -zn;

    // the camera frame expressed in world frame
    vec3 camera_x = vec3(view.x.x, view.y.x, view.z.x);
    vec3 camera_y = vec3(view.x.y, view.y.y, view.z.y);
    vec3 camera_z = vec3(view.x.z, view.y.z, view.z.z);

    *dir = camera_x * xv + camera_y * yv + camera_z * zv;
    *dir = normalize(*dir);
    *pos = camera_x * (-view.w.x) + camera_y * (-view.w.y) + camera_z * (-view.w.z);
}

// bool intersect_ray_box(vec3 ro, vec3 rd, vec3 )

void tick(Input io, float t, float dt)
{
    mat4 projection = mat_perspective(PI / 4.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.1f, 10.0f);
    mat4 view = mat_translate(0.0f, 0.0f, -5.0f) * mat_rotate_x(0.3f) * mat_rotate_y(0.2f);

    vec2 ndc = (io.mouse.pos / vec2(WINDOW_WIDTH, -WINDOW_HEIGHT)) * 2.0f + vec2(-1.0f, +1.0f);
    vec3 dir, pos;
    pick_vector_perspective(ndc, 0.1f, projection, view, &dir, &pos);

    float tt = -pos.y / dir.y;
    vec3 traced = pos + dir * tt;

    mat4 model = mat_translate(traced) * mat_scale(0.5f);

    // SceneObject *selected = scene_tick(projection, view, io, dt);

    // mat4 model = mat_translate(selected->position) *
    //              mat_rotate_z(selected->rotation.euler.z) *
    //              mat_rotate_y(selected->rotation.euler.y) *
    //              mat_rotate_x(selected->rotation.euler.x) *
    //              mat_scale(selected->scale);

    depth_test(true, GL_LEQUAL);
    depth_write(true);
    clear(0.02f, 0.05f, 0.1f, 1.0f, 1.0f);
    begin(&pass);
    glBindBuffer(GL_ARRAY_BUFFER, cube.vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.ibo);
    attribfv("position", 3, 6, 0);
    attribfv("normal", 3, 6, 3);
    uniformf("projection", projection);
    uniformf("view", view);
    uniformf("model", model);
    glDrawElements(GL_TRIANGLES, cube.index_count, cube.index_type, 0);

    // scene_present_gui(projection, view);

    ImGui::NewFrame();
    ImGui::Begin("Scene");
    char text[256];
    sprintf(text, "%.2f %.2f %.2f", dir.x, dir.y, dir.z);
    ImGui::Text(text);
    ImGui::End();
    ImGui::Render();
}

#include "sumo.cpp"
