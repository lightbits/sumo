#define __introspect
#include <stdio.h>
#include <stdint.h>
typedef float       r32;
typedef uint64_t    u64;
typedef uint32_t    u32;
typedef uint16_t    u16;
typedef uint8_t     u08;
typedef int32_t     s32;
typedef int16_t     s16;
typedef int8_t      s08;

__introspect struct Quaternion
{
    float x, y, z, w;
};

__introspect struct Vector3
{
    float x, y, z;
};

struct EntityFamiliar
{
    Quaternion rotation;
    Vector3 offset;
    float magic_power;
};

__introspect struct Entity
{
    // EntityFamiliar familiar;
    Quaternion rotation;
    Vector3 position;
    bool is_alive;
    float size;
    u32 id;
};

enum MetaType
{
    MetaType_float,
    MetaType_Quaternion,
    MetaType_Vector3,
    MetaType_bool,
    MetaType_u32
};

struct MetaMemberData
{
    MetaType type;
    char *name;
    u32 offset;
};

#define array_count(array) (sizeof(array) / sizeof(array[0]))

#include "../build/generated_meta_code.h"

void debug_dump_member_data(void *struct_ptr,
                            MetaMemberData *member_data,
                            u32 member_count)
{
    for (u32 member_index = 0;
         member_index < member_count;
         member_index++)
    {
        MetaMemberData member = member_data[member_index];
        void *member_ptr = ((u08*)struct_ptr) + member.offset;
        switch (member.type)
        {
            case MetaType_float:
            {
                printf("%s = %.2f\n", member.name, *(float*)member_ptr);
            } break;

            case MetaType_Quaternion:
            {
                printf("%s = {%.2f %.2f %.2f %.2f}\n", member.name,
                       ((Quaternion*)member_ptr)->x,
                       ((Quaternion*)member_ptr)->y,
                       ((Quaternion*)member_ptr)->z,
                       ((Quaternion*)member_ptr)->w);

            } break;

            case MetaType_Vector3:
            {
                printf("%s = {%.2f %.2f %.2f}\n", member.name,
                       ((Vector3*)member_ptr)->x,
                       ((Quaternion*)member_ptr)->y,
                       ((Quaternion*)member_ptr)->z);
            } break;

            case MetaType_bool:
            {
                printf("%s = %s\n", member.name,
                       (*(bool*)member_ptr) ? "true" : "false");
            } break;

            case MetaType_u32:
            {
                printf("%s = %u\n", member.name,
                       *(u32*)member_ptr);
            } break;

        }
    }
}

int main()
{
    // fill in the entities
    Entity entity = {};
    entity.rotation.x = 1;
    entity.rotation.y = 2;
    entity.rotation.z = 3;
    entity.rotation.w = 4;

    entity.position.x = 10;
    entity.position.y = 20;
    entity.position.z = 30;

    entity.is_alive = true;
    entity.size = 3.1415926f;
    entity.id = 123456;

    debug_dump_member_data(&entity,
                           member_of_Entity,
                           array_count(member_of_Entity));

    Quaternion quat = {};
    quat.x = 5;
    quat.y = 4;
    quat.z = 2;
    quat.w = 1;
    debug_dump_member_data(&quat,
                           member_of_Quaternion,
                           array_count(member_of_Quaternion));
}

#if 0
#include "sumo.h"
#include <stdio.h>
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

#define array_count(array) (sizeof(array) / sizeof(array[0]))

#include "../build/generated_meta_code.h"

void debug_dump_member_data(void *struct_ptr,
                            MetaMemberData *member_data,
                            u32 member_count)
{
    for (u32 member_index = 0;
         member_index < member_count;
         member_index++)
    {
        MetaMemberData member = member_data[member_index];
        void *member_ptr = ((u08*)struct_ptr) + member.offset;
        switch (member.type)
        {
            case MetaType_float:
            {
                printf("%s = %.2f\n", member.name, *(float*)member_ptr);
            } break;

            case MetaType_Quaternion:
            {
                printf("%s = {%.2f %.2f %.2f %.2f}\n", member.name,
                       ((Quaternion*)member_ptr)->x,
                       ((Quaternion*)member_ptr)->y,
                       ((Quaternion*)member_ptr)->z,
                       ((Quaternion*)member_ptr)->w);

            } break;

            case MetaType_Vector3:
            {
                printf("%s = {%.2f %.2f %.2f}\n", member.name,
                       ((Vector3*)member_ptr)->x,
                       ((Quaternion*)member_ptr)->y,
                       ((Quaternion*)member_ptr)->z);
            } break;

            case MetaType_bool:
            {
                printf("%s = %s\n", member.name,
                       (*(bool*)member_ptr) ? "true" : "false");
            } break;

            case MetaType_u32:
            {
                printf("%s = %u\n", member.name,
                       *(u32*)member_ptr);
            } break;

        }
    }
}

void init()
{
    // fill in the entities
    Entity entity = {};
    entity.rotation.x = 1;
    entity.rotation.y = 2;
    entity.rotation.z = 3;
    entity.rotation.w = 4;

    entity.position.x = 10;
    entity.position.y = 20;
    entity.position.z = 30;

    entity.is_alive = true;
    entity.size = 3.1415926f;
    entity.id = 123456;

    debug_dump_member_data(&entity,
                           member_of_Entity,
                           array_count(member_of_Entity));

    Quaternion quat = {};
    quat.x = 5;
    quat.y = 4;
    quat.z = 2;
    quat.w = 1;
    debug_dump_member_data(&quat,
                           member_of_Quaternion,
                           array_count(member_of_Quaternion));
}

void tick(Input io, float t, float dt)
{
    clearc(0.35f, 0.55f, 1.0f, 1.0f);

    // Get the introspection meta data
    // Dump it to the ImGui view

    ImGui::NewFrame();
    ImGui::Begin("Diffuse Shader");
    ImGui::End();
    ImGui::Render();
}

#include "sumo.cpp"

#endif
