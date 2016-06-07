#define USE_NEW_MATH
#include "sumo.h"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MULTISAMPLES 4
#define WINDOW_FLAGS SDL_WINDOW_RESIZABLE
#define GLSL(src) "#version 150\n" #src

char *vs = GLSL(
in vec3 position;
in vec3 normal;
in vec2 coord;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
out vec2 vs_coord;
out vec3 vs_normal;
void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0);
    vs_normal = normal;
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
    r32 r = 0.5f+0.4f*sin(TWO_PI*x)*sin(TWO_PI*y)*sin(TWO_PI*z);
    r32 d = x*x+y*y+z*z-r*r;
    return d;
}

vec3 GRADIENT(vec3 p)
{
    r32 d = 0.01f;
    r32 dx = POTENTIAL(p.x+d, p.y, p.z)-POTENTIAL(p.x-d, p.y, p.z);
    r32 dy = POTENTIAL(p.x, p.y+d, p.z)-POTENTIAL(p.x, p.y-d, p.z);
    r32 dz = POTENTIAL(p.x, p.y, p.z+d)-POTENTIAL(p.x, p.y, p.z-d);
    return m_normalize(m_vec3(dx, dy, dz));
}

void init()
{
    rp = make_render_pass(vs, fs);

    u08 E1[12] = { 0, 1, 2, 3, 3, 0, 1, 2, 4, 5, 6, 7 };
    u08 E2[12] = { 1, 2, 3, 0, 4, 5, 6, 7, 5, 6, 7, 4 };
    u16 E[256]; // Maps potential-negativity-mask to which edges are intersected
    vec3 INDEX_TO_VERTEX[8] = {
        m_vec3(-1, -1, -1),
        m_vec3(+1, -1, -1),
        m_vec3(+1, -1, +1),
        m_vec3(-1, -1, +1),
        m_vec3(-1, +1, +1),
        m_vec3(-1, +1, -1),
        m_vec3(+1, +1, -1),
        m_vec3(+1, +1, +1)
    };
    for (u32 mask = 0; mask < 256; mask++)
    {
        E[mask] = 0;
        for (u32 e = 0; e < 12; e++)
        {
            u08 e1 = E1[e];
            u08 e2 = E2[e];
            #define BIT_IS_SET(I) (mask & (1 << (I)))
            if ((BIT_IS_SET(e1) && !BIT_IS_SET(e2)) ||
                (!BIT_IS_SET(e1) && BIT_IS_SET(e2)))
            {
                E[mask] |= 1 << e;
            }
        }
        // if (mask % 10 == 0)
        //     printf("\n");
        // printf("0x%04x, ", E[mask]);
    }

    #define N 65
    static bool S[N*N*N];
    static u08 M[N*N*N];
    static int I[N*N*N];
    static vec3 C[N*N*N];
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
            r32 f[] = {f111, f211, f212, f112, f122, f121, f221, f222};

            u08 mask = 0;
            if (f111 < 0.0f) mask |= 1;
            if (f211 < 0.0f) mask |= 2;
            if (f212 < 0.0f) mask |= 4;
            if (f112 < 0.0f) mask |= 8;
            if (f122 < 0.0f) mask |= 16;
            if (f121 < 0.0f) mask |= 32;
            if (f221 < 0.0f) mask |= 64;
            if (f222 < 0.0f) mask |= 128;

            u16 edges = E[mask];
            vec3 offset = m_vec3(0.0f);
            int hits = 0;
            for (u32 e = 0; e < 12; e++)
            {
                if ((edges & (1 << e)) == 0)
                    continue;

                u08 i1 = E1[e];
                u08 i2 = E2[e];
                vec3 p1 = INDEX_TO_VERTEX[i1];
                vec3 p2 = INDEX_TO_VERTEX[i2];
                r32 f1 = f[i1];
                r32 f2 = f[i2];

                r32 t = m_clamp(f1 / (f1 - f2), 0.0f, 1.0f);
                vec3 p = p1 + (p2 - p1) * t;
                offset += p;
                hits++;
            }
            offset /= (r32)hits;

            int i = zi*N*N+yi*N+xi;
            if (mask != 0 && mask != 0xFF)
            {
                I[num_centroids++] = i;
                C[i] = offset;
                M[i] = mask;
            }
            else
            {
                M[i] = 0;
            }
        }
    }

    struct vertex
    {
        vec3 p;
        vec3 n;
        r32 u;
        r32 v;
    };
    u32 sizeof_data = (num_centroids*3*6)*sizeof(vertex);
    vertex *data = (vertex*)malloc(sizeof_data);
    num_indices = 0;
    vertex *ptr = data;

    for (int i = 0; i < num_centroids; i++)
    {
        int xi = (I[i] % (N)) / 1;
        int yi = (I[i] % (N*N)) / N;
        int zi = (I[i] % (N*N*N)) / (N*N);

        #define OFFSET(XI, YI, ZI) C[(ZI)*N*N+(YI)*N+(XI)]
        #define TOWORLD(I) (-1.0f + 2.0f*(I)/N)
        #define CENTROID(XI, YI, ZI) (m_vec3(TOWORLD(XI), TOWORLD(YI), TOWORLD(ZI)) + OFFSET(XI, YI, ZI)*(1.0f/N))
        vec3 p000 = CENTROID(xi, yi, zi);     // center
        vec3 p100 = CENTROID(xi-1, yi, zi);   // left
        vec3 p010 = CENTROID(xi, yi-1, zi);   // down
        vec3 p001 = CENTROID(xi, yi, zi-1);   // back
        vec3 p110 = CENTROID(xi-1, yi-1, zi); // left-down
        vec3 p101 = CENTROID(xi-1, yi, zi-1); // left-back
        vec3 p011 = CENTROID(xi, yi-1, zi-1); // back-down

        vec3 n000 = GRADIENT(p000);
        vec3 n100 = GRADIENT(p100);
        vec3 n010 = GRADIENT(p010);
        vec3 n001 = GRADIENT(p001);
        vec3 n110 = GRADIENT(p110);
        vec3 n101 = GRADIENT(p101);
        vec3 n011 = GRADIENT(p011);

        #define MASK(X, Y, Z) M[(Z)*N*N+(Y)*N+(X)]
        u08 m000 = MASK(xi, yi, zi);
        u08 m100 = MASK(xi-1, yi, zi);
        u08 m010 = MASK(xi, yi-1, zi);
        u08 m001 = MASK(xi, yi, zi-1);
        u08 m110 = MASK(xi-1, yi-1, zi);
        u08 m101 = MASK(xi-1, yi, zi-1);
        u08 m011 = MASK(xi, yi-1, zi-1);
        bool ccw = (m000 & 1) != 0;

        #define PUSH_VERT(P, N, U, V) { ptr->p = P; ptr->n = N; ptr->u = U; ptr->v = V; ptr++; num_indices++; }
        #define EMIT_QUAD(P00, P10, P11, P01, N00, N10, N11, N01) \
            PUSH_VERT(P00, N00, 0.0f, 0.0f); \
            PUSH_VERT(P10, N10, 1.0f, 0.0f); \
            PUSH_VERT(P11, N11, 1.0f, 1.0f); \
            PUSH_VERT(P11, N11, 1.0f, 1.0f); \
            PUSH_VERT(P01, N01, 0.0f, 1.0f); \
            PUSH_VERT(P00, N00, 0.0f, 0.0f);

        if (m100 && m001 && m101)
        {
            if (ccw) { EMIT_QUAD(p000, p001, p101, p100, n000, n001, n101, n100); }
            else     { EMIT_QUAD(p000, p100, p101, p001, n000, n100, n101, n001); }
        }

        if (m100 && m010 && m110)
        {
            if (ccw) { EMIT_QUAD(p000, p100, p110, p010, n000, n100, n110, n010); }
            else     { EMIT_QUAD(p000, p010, p110, p100, n000, n010, n110, n100); }
        }

        if (m001 && m010 && m011)
        {
            if (ccw) { EMIT_QUAD(p000, p010, p011, p001, n000, n010, n011, n001); }
            else     { EMIT_QUAD(p000, p001, p011, p010, n000, n001, n011, n010); }
        }
    }

    buf = make_buffer(GL_ARRAY_BUFFER, num_indices*sizeof(vertex), data, GL_STATIC_DRAW);
}

void tick(Input io, float t, float dt)
{
    glViewport(0, 0, io.window_width, io.window_height);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    depth_test(true);
    clear(1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    mat4 projection = mat_perspective(PI / 4.0f, io.window_width, io.window_height, 0.1f, 10.0f);
    mat4 view = mat_translate(0.0f, 0.0f, -4.0f) * camera_holdclick(io, dt);
    mat4 model = mat_scale(1.0f);
    begin(&rp);
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    attribfv("position", 3, 8, 0);
    attribfv("normal", 3, 8, 3);
    attribfv("coord", 2, 8, 6);
    uniformf("projection", projection);
    uniformf("view", view);
    uniformf("model", model);
    glDrawArrays(GL_TRIANGLES, 0, num_indices);
}

#include "sumo.cpp"
