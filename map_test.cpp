#include "prototype.h"
#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 300
#define WINDOW_FLAGS SDL_WINDOW_BORDERLESS

const u32 map_hash_coefficient_count = 257;
const u32 map_hash_coefficients[map_hash_coefficient_count] = {
    108, 5, 221, 101, 133,
    42, 141, 179, 219, 125,
    230, 213, 55, 8, 40,
    132, 86, 81, 192, 21,
    17, 186, 35, 152, 119,
    237, 253, 16, 85, 135,
    221, 126, 64, 87, 138,
    161, 14, 133, 191, 139,
    224, 46, 97, 233, 233,
    74, 248, 10, 170, 162,
    222, 146, 187, 4, 133,
    50, 35, 81, 193, 185,
    204, 115, 47, 107, 218,
    30, 184, 189, 205, 125,
    158, 230, 142, 186, 189,
    252, 199, 33, 148, 173,
    162, 6, 8, 15, 168,
    204, 32, 217, 22, 136,
    183, 52, 47, 130, 215,
    215, 191, 202, 245, 142,
    73, 121, 157, 65, 71,
    150, 173, 129, 160, 154,
    83, 207, 10, 59, 92,
    230, 173, 102, 185, 4,
    239, 133, 171, 8, 215,
    119, 48, 183
};

u32 map_hash(char *s)
{
    u32 result = 0;
    char *c = s;
    u32 ip = 0;
    while (*c)
    {
        result += *c * map_hash_coefficients[ip];
        ip = (ip + 1) % map_hash_coefficient_count;
        c++;
    }
    return result;
}

#if 0
struct map_node_t;
struct map_node_t
{
    map_node_t *next;
    map_node_t *prev;
};

struct map_t
{
    char *keys;
    u08  *data;
    u32   size;
    u32   used;
};

void map_set(map_t *m, char *key, u08 *value, u32 count)
{
    u32 h = map_hash(key);
}
#endif

void init()
{
    u32 h = map_hash("hello I am a key");
    printf("h = %d\n", h);
}

void tick(float t, float dt)
{

}

#include "prototype.cpp"
