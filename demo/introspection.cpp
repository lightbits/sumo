#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS
#include "some_include"
#include <another_include>
struct Hello
{
    vec3 member;
    // a comment
    /* a multi
    line
    comment
    vec3
    */
    char * a_pointer;
    vec2 thing;
};

static vec3 x;
void foo(char *bar)
{
    printf("a string\n");
}

/*#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

// This is a comment to confuse the pre-processor

#define __introspect

__introspect struct Quaternion
{
    float x, y, z, w;
};

__introspect struct Vector3
{
    float x, y, z;
};

__introspect struct Entity
{
    Quaternion rotation;
    Vector3 position;
    bool is_alive;
    float size;
    u32 id;
};

Entity entities[NUM_ENTITIES];

void init()
{
    // fill in the entities
}

void tick(Input io, float t, float dt)
{
    clearc(0.35f, 0.55f, 1.0f, 1.0f);

    // Select an entity

    // Get the introspection meta data
    // Dump it to the ImGui view

    ImGui::NewFrame();
    ImGui::Begin("Diffuse Shader");
    ImGui::ColorEdit4("lightColor", lightColor);
    ImGui::SliderFloat("attenuation", &attenuation, 1.0f, 16.0f);
    ImGui::End();
    ImGui::Render();
}

#include "sumo.cpp"
*/
