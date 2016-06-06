#define USE_NEW_MATH
#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_RESIZABLE
#define GLSL(src) "#version 150\n" #src

char *vs = GLSL(
in vec3 position;
in vec2 coord;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
out vec2 vs_coord;
out vec3 vs_normal;
void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
    vs_normal = position;
    vs_coord = coord;
}
);

char *fs = GLSL(
in vec2 vs_coord;
in vec3 vs_normal;
out vec4 fs_color;

float WIREFRAME(float width)
{
    float s = vs_coord.x;
    float t = vs_coord.y;
    float wx = width*fwidth(s);
    float wy = width*fwidth(t);
    // float h = 1.0 - (smoothstep(0.0, wx, s) - smoothstep(1.0 - wx, 1.0, s));
    // float v = 1.0 - (smoothstep(0.0, wy, t) - smoothstep(1.0 - wy, 1.0, t));
    // return clamp(h + v, 0.0, 1.0);
    float hs = (smoothstep(0.0, wx, s)-smoothstep(1.0-wx, 1.0, s));
    float ht = (smoothstep(0.0, wy, t)-smoothstep(1.0-wy, 1.0, t));
    return 1.0-hs*ht;
}

vec3 LIGHT(vec3 n, vec3 l, vec3 c)
{
    float ndotl = max(dot(n, l), 0.0);
    return c * ndotl;
}

void main()
{
    vec3 n = normalize(vs_normal);

    vec3 albedo = vec3(0.65, 0.63, 0.6);
    vec3 color = vec3(0.0);
    color += LIGHT(n, normalize(vec3(-0.3, 1.0, -0.2)), vec3(1.3, 1.0, 0.6) * 0.2);
    color += LIGHT(n, normalize(vec3(1.0)), vec3(1.3, 1.2, 0.8) * 1.2);
    color += LIGHT(n, normalize(vec3(-1.0, -1.0, 0.2)), vec3(0.72, 0.85, 0.93) * 0.4);
    color *= albedo;

    float wireframe = WIREFRAME(1.0);
    color = mix(color, vec3(1.7, 1.6, 2.0) * color, 0.2 * wireframe);
    fs_color = vec4(color, 1.0);
}
);

u32 buf;
RenderPass rp;
u32 num_indices;

r32 POTENTIAL(r32 x, r32 y, r32 z)
{
    // return m_min(m_min(m_min(x, y), z),
    //            sqrt(x*x+y*y+z*z) - 0.3f);
    // return sqrt(x*x+y*y+z*z) - 0.8f;
    r32 d1 = y+0.1f*sin(TWO_PI*x)*sin(TWO_PI*z);
    r32 d2 = sqrt(x*x+y*y+z*z) - 0.5f;
    return m_min(d1, d2);
}

void init()
{
    rp = make_render_pass(vs, fs);

    struct vertex
    {
        vec3 p;
        r32 u;
        r32 v;
    };

    #if 1
    #define N 65
    static bool S[N*N*N];
    static int I[N*N*N];
    int num_centroids = 0;
    {
        for (int zi = 0; zi < N; zi++)
        for (int yi = 0; yi < N; yi++)
        for (int xi = 0; xi < N; xi++)
        {
            #define EVALI(XI, YI, ZI) POTENTIAL(-1.0f + 2.0f*(XI)/N, -1.0f + 2.0f*(YI)/N, -1.0f + 2.0f*(ZI)/N)
            r32 f111 = EVALI(xi, yi, zi);
            r32 f211 = EVALI(xi+1, yi, zi);
            r32 f221 = EVALI(xi+1, yi+1, zi);
            r32 f121 = EVALI(xi, yi+1, zi);
            r32 f112 = EVALI(xi, yi, zi+1);
            r32 f212 = EVALI(xi+1, yi, zi+1);
            r32 f222 = EVALI(xi+1, yi+1, zi+1);
            r32 f122 = EVALI(xi, yi+1, zi+1);

            u08 mask = 0;
            if (f111 < 0.0f) mask |= 1;
            if (f211 < 0.0f) mask |= 2;
            if (f221 < 0.0f) mask |= 4;
            if (f121 < 0.0f) mask |= 8;
            if (f112 < 0.0f) mask |= 16;
            if (f212 < 0.0f) mask |= 32;
            if (f222 < 0.0f) mask |= 64;
            if (f122 < 0.0f) mask |= 128;

            if (mask != 0 && mask != 0xFF)
            {
                I[num_centroids++] = zi*N*N+yi*N+xi;
                S[zi*N*N+yi*N+xi] = true;
            }
            else
            {
                S[zi*N*N+yi*N+xi] = false;
            }
        }
    }

    u32 sizeof_data = (num_centroids*3*6)*sizeof(vertex);
    vertex *data = (vertex*)malloc(sizeof_data);
    num_indices = 0;
    vertex *ptr = data;

    for (int i = 0; i < num_centroids; i++)
    {
        int xi = (I[i] % (N)) / 1;
        int yi = (I[i] % (N*N)) / N;
        int zi = (I[i] % (N*N*N)) / (N*N);

        #define NDC(I) (-1.0f + 2.0f*(I+0.5f)/N)
        #define POSITION(XI, YI, ZI) m_vec3(NDC(XI), NDC(YI), NDC(ZI))
        vec3 p000 = POSITION(xi, yi, zi);     // center
        vec3 p100 = POSITION(xi-1, yi, zi);   // left
        vec3 p010 = POSITION(xi, yi-1, zi);   // down
        vec3 p001 = POSITION(xi, yi, zi-1);   // back
        vec3 p110 = POSITION(xi-1, yi-1, zi); // left-down
        vec3 p101 = POSITION(xi-1, yi, zi-1); // left-back
        vec3 p011 = POSITION(xi, yi-1, zi-1); // back-down

        #define IS_SURFACE(X, Y, Z) S[(Z)*N*N+(Y)*N+(X)]
        bool s100 = IS_SURFACE(xi-1, yi, zi);
        bool s010 = IS_SURFACE(xi, yi-1, zi);
        bool s001 = IS_SURFACE(xi, yi, zi-1);
        bool s110 = IS_SURFACE(xi-1, yi-1, zi);
        bool s101 = IS_SURFACE(xi-1, yi, zi-1);
        bool s011 = IS_SURFACE(xi, yi-1, zi-1);

        #define PUSH_VERT(P, U, V) { ptr->p = P; ptr->u = U; ptr->v = V; ptr++; num_indices++; }
        #define EMIT_QUAD(P00, P10, P11, P01) { \
            PUSH_VERT(P00, 0.0f, 0.0f); \
            PUSH_VERT(P10, 1.0f, 0.0f); \
            PUSH_VERT(P11, 1.0f, 1.0f); \
            PUSH_VERT(P11, 1.0f, 1.0f); \
            PUSH_VERT(P01, 0.0f, 1.0f); \
            PUSH_VERT(P00, 0.0f, 0.0f); }

        if (s100 && s001 && s101)
        {
            EMIT_QUAD(p000, p100, p101, p001);
        }

        if (s100 && s010 && s110)
        {
            EMIT_QUAD(p000, p100, p110, p010);
        }

        if (s001 && s010 && s011)
        {
            EMIT_QUAD(p000, p001, p011, p010);
        }
    }

    buf = make_buffer(GL_ARRAY_BUFFER, num_indices*sizeof(vertex), data, GL_STATIC_DRAW);

    #else
    #define N 16
    #define NUM_VERTICES (N*N*6)
    vertex data[NUM_VERTICES];
    vertex *ptr = data;
    for (int y = 0; y < N; y++)
    for (int x = 0; x < N; x++)
    {
        vec3 p11 = m_normalize(m_vec3(-1.0f + 2.0f*x/N, -1.0f + 2.0f*y/N, 1.0f));
        vec3 p12 = m_normalize(m_vec3(-1.0f + 2.0f*(x+1)/N, -1.0f + 2.0f*y/N, 1.0f));
        vec3 p21 = m_normalize(m_vec3(-1.0f + 2.0f*x/N, -1.0f + 2.0f*(y+1)/N, 1.0f));
        vec3 p22 = m_normalize(m_vec3(-1.0f + 2.0f*(x+1)/N, -1.0f + 2.0f*(y+1)/N, 1.0f));

        #define PUSH1(P, U, V) { vertex v = {P, U, V}; *ptr = v; ptr++; }
        PUSH1(p11, 0.0f, 0.0f);
        PUSH1(p12, 1.0f, 0.0f);
        PUSH1(p22, 1.0f, 1.0f);
        PUSH1(p22, 1.0f, 1.0f);
        PUSH1(p21, 0.0f, 1.0f);
        PUSH1(p11, 0.0f, 0.0f);
    }
    buf = make_buffer(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    #endif
}

void tick(Input io, float t, float dt)
{
    glViewport(0, 0, io.window_width, io.window_height);
    depth_test(true);
    clear(0.35f, 0.55f, 1.0f, 1.0f, 1.0f);
    mat4 projection = mat_perspective(PI / 4.0f, io.window_width, io.window_height, 0.1f, 10.0f);
    mat4 view = mat_translate(0.0f, 0.0f, -4.0f) * camera_holdclick(io, dt);
    mat4 model = mat_scale(1.0f);
    begin(&rp);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    attribfv("position", 3, 5, 0);
    attribfv("coord", 2, 5, 3);
    uniformf("projection", projection);
    uniformf("view", view);
    uniformf("model", model);
    glDrawArrays(GL_TRIANGLES, 0, num_indices);
}

#include "sumo.cpp"
