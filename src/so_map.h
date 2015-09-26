/* so_map - v0.1

A sorted lookup table that contains values of arbitrary type,
with char* strings as key. Lookup is done by binary search.

Changelog
=========
26. september 2015
    Converted to so code format for code reuse purposes.

How to compile
====================================================================
This file contains both the header file and the implementation file.
To create the implementation in one source file include this header:

    #define SO_MAP_IMPLEMENTATION
    #include "so_map.h"

TODO
====================================================================
* Custom allocator.
* Variable constant allocation size.

Self-contained example
====================================================================
#define SO_MAP_IMPLEMENTATION
#include "so_map.h"
typedef Map(float) Map_float;
int main(int argc, char **argv)
{
    Map_float a = {};
    map_alloc(&a, 16);
    map_set(&a, "acb", 4.4f);
    map_set(&a, "ab", 3.3f);
    map_set(&a, "aa", 2.2f);
    map_set(&a, "a", 1.1f);
    for (int i = 0; i < map_len(&a); i++)
    {
        char *key = a.base.keys[i];
        float *x = map_get(&a, key, float);
        if (x) printf("%d : %s = %f\n", i, key, *x);
    }
    map_free(&a);
    return 0;
}
*/

#ifndef SO_MAP_HEADER_INCLUDE
#define SO_MAP_HEADER_INCLUDE

struct MapBase
{
    char **keys;
    unsigned char *data;
    unsigned int  size;
    unsigned int  used;
};

#define map_alloc(s, count) \
    soi_map_alloc(&(s)->base, count, sizeof((s)->tmp));

#define map_free(s) \
    soi_map_free(&(s)->base);

#define Map(T) \
    struct { MapBase base; T tmp; }

#define map_set(s, key, value) \
    (s)->tmp = (value); \
    soi_map_set(&(s)->base, (key), (unsigned char*)&(s)->tmp, sizeof((s)->tmp));

#define map_get(s, key, type) \
    (type*)soi_map_get(&(s)->base, (key), sizeof(type));

#define map_len(s) (s)->base.used

extern void *soi_map_get(MapBase *s, char *key, unsigned int size);
extern void soi_map_set(MapBase *s, char *key, unsigned char *value, unsigned int size);
extern void soi_map_alloc(MapBase *base, unsigned int count, unsigned int element_size);
extern void soi_map_free(MapBase *base);

#endif // SO_MAP_HEADER_INCLUDE
#ifdef SO_MAP_IMPLEMENTATION

// return: true and the index of the matching element if found.
//         false and the index to insert the key to preserve
//         lexicographic ordering, if not found.
bool soi_binary_search(char **keys, int n, char *key, int *result)
{
    if (n == 0)
    {
        *result = 0;
        return false;
    }
    int lo = 0;
    int hi = n - 1;
    while (hi - lo > 0)
    {
        int mid = lo + (hi - lo) / 2;
        int cmp = strcmp(key, keys[mid]);
        if (cmp < 0)
        {
            hi = mid;
        }
        else if (cmp > 0)
        {
            lo = mid + 1;
        }
        else
        {
            *result = mid;
            return true;
        }
    }
    int cmp = strcmp(key, keys[lo]);
    if (cmp < 0)
        *result = lo;
    else if (cmp > 0)
        *result = lo + 1;
    else
        *result = hi;
    return cmp == 0;
}

void soi_map_alloc(MapBase *base, unsigned int count, unsigned int element_size)
{
    base->keys = (char**)malloc(count);
    base->data = (unsigned char*)malloc(count * element_size);
    base->size = count;
    base->used = 0;
}

void soi_map_free(MapBase *base)
{
    for (unsigned int i = 0; i < base->used; i++)
        free(base->keys[i]);
    free(base->keys);
    free(base->data);
    base->size = 0;
    base->used = 0;
}

void soi_map_set(MapBase *s, char *key, unsigned char *value, unsigned int size)
{
    assert(s->used < s->size);
    int insert_at;
    if (soi_binary_search(s->keys, s->used, key, &insert_at))
        return; // Item already exists
    for (int i = s->used; i > insert_at; i--)
    {
        s->keys[i] = s->keys[i - 1];
        memcpy(s->data + i * size, s->data + (i - 1) * size, size);
    }
    char *new_key = (char*)malloc(strlen(key) + 1);
    assert(new_key);
    strcpy(new_key, key);
    s->keys[insert_at] = new_key;
    s->used++;
    memcpy(s->data + insert_at * size, value, size);
}

void *soi_map_get(MapBase *s, char *key, unsigned int size)
{
    int i;
    bool exists = soi_binary_search(s->keys, s->used, key, &i);
    if (exists)
        return s->data + size * i;
    else
        return 0;
}

#endif // SO_MAP_IMPLEMENTATION
