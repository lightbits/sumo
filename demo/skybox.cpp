#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

Mesh cube;
GLuint skybox;

void init()
{
    cube = make_cube();
    // skybox = so_load_tex2d("")
}

void tick(float t, float dt)
{
    clearc(0.35f, 0.55f, 1.0f, 1.0f);
}

#include "sumo.cpp"
