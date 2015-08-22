/*
TODO
===============
* Custom allocator.
* Variable constant allocation size.

Usage
===============
Define your array with a contained type with
    typedef map(<your type>) my_map_t

The stored values are sorted by their key, which
is always a character string.
*/

#ifndef _map_h_
#define _map_h_
#include <stdint.h>
typedef float       r32;
typedef uint64_t    u64;
typedef uint32_t    u32;
typedef uint16_t    u16;
typedef uint8_t     u08;
typedef int32_t     s32;
typedef int16_t     s16;
typedef int8_t      s08;

struct MapBase
{
    char **keys;
    u08  *data;
    u32   size;
    u32   used;
};

#define map_alloc(s, count) \
    map_alloc_(&(s)->base, count, sizeof((s)->tmp));

#define map_free(s) \
    map_free_(&(s)->base);

#define Map(T) \
    struct { MapBase base; T tmp; }

#define map_set(s, key, value) \
    (s)->tmp = (value); \
    map_set_(&(s)->base, (key), (u08*)&(s)->tmp, sizeof((s)->tmp));

#define map_get(s, key, type) \
    (type*)map_get_(&(s)->base, (key), sizeof(type));

#define map_len(s) (s)->base.used

void *map_get_(MapBase *s, char *key, u32 size);
void map_set_(MapBase *s, char *key, u08 *value, u32 size);
void map_alloc_(MapBase *base, u32 count, u32 element_size);
void map_free_(MapBase *base);

typedef Map(r32) Map_r32;
typedef Map(u32) Map_u32;
typedef Map(u16) Map_u16;
typedef Map(u08) Map_u08;
typedef Map(s32) Map_s32;
typedef Map(s16) Map_s16;
typedef Map(s08) Map_s08;

#endif
