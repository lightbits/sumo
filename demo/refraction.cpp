#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

// GLuint make_random_blob()
// {
//     const u32 n = 16;
//     float v[]
//     GLuint result = make_buffer(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
// }

Mesh cube;
GLuint skybox;

void init()
{
    cube = make_cube();
}

void tick(Input io, float t, float dt)
{
    clearc(0.35f, 0.55f, 1.0f, 1.0f);
}

#include "sumo.cpp"
