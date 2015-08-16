/*
TODO
===============
* Custom allocator.
* Variable constant allocation size.


Usage
===============
Define your array with a contained type with
    typedef sorted_array(<your type>) my_sorted_array_t

In this file are defined a few
    sorted_u32_array_t - unsigned int
    sorted_u16_array_t - unsigned short
    sorted_u08_array_t - unsigned char
    sorted_s32_array_t - int
    sorted_s16_array_t - short
    sorted_s08_array_t - char

The stored values are sorted by their key, which
is always a character string.
*/

#ifndef _sorted_array_h_
#define _sorted_array_h_
#include <stdint.h>
typedef float       r32;
typedef uint64_t    u64;
typedef uint32_t    u32;
typedef uint16_t    u16;
typedef uint8_t     u08;
typedef int32_t     s32;
typedef int16_t     s16;
typedef int8_t      s08;

#if 0
#define Sorted_Array_Size 512

struct SortedArrayBase
{
    char *keys[Sorted_Array_Size];
    int count;
};

#define sorted_array(T)                   \
    struct {                              \
        SortedArrayBase base;         \
        T data[Sorted_Array_Size];        \
        T tmp;                            \
    }

#define sorted_array_set(s, key, value)   \
    (s)->tmp = (value);                   \
    sorted_array_set_(&(s)->base,         \
                      key,                \
                      (u08*)&((s)->data), \
                      (u08*)&((s)->tmp),  \
                      sizeof((s)->tmp));


#define sorted_array_get(s, key, T)       \
    (T*)sorted_array_get_(&(s)->base,     \
                         (u08*)(s)->data, \
                          key, sizeof(T)) \

void*
sorted_array_get_(SortedArrayBase *s,
                  u08 *data,
                  char *key,
                  u32 size);

void
sorted_array_set_(SortedArrayBase *s,
                  char *key,
                  u08 *data,
                  u08 *value,
                  u32 size);
#else
struct SortedArrayBase
{
    char **keys;
    u08  *data;
    u32   size;
    u32   used;
};

#define sorted_array_alloc(s, count)      \
    sorted_array_alloc_(&(s)->base,       \
                        count,            \
                        sizeof((s)->tmp));

#define free_sorted_array(s) \
    free_sorted_array_(&(s)->base);

#define SortedArray(T) \
    struct { SortedArrayBase base; T tmp; }

#define sorted_array_set(s, key, value)  \
    (s)->tmp = (value);                  \
    sorted_array_set_(&(s)->base, (key), \
                      (u08*)&(s)->tmp,   \
                      sizeof((s)->tmp));

#define sorted_array_get(s, key, type)          \
    (type*)sorted_array_get_(&(s)->base, (key), \
                             sizeof(type));

void *sorted_array_get_(SortedArrayBase *s,
                        char *key, u32 size);

void sorted_array_set_(SortedArrayBase *s,
                       char *key, u08 *value,
                       u32 size);

void sorted_array_alloc_(SortedArrayBase *base,
                         u32 count, u32 element_size);

void free_sorted_array_(SortedArrayBase *base);

#define sorted_array_len(s) (s)->base.used

#endif
typedef SortedArray(r32) SortedArray_r32;
typedef SortedArray(u32) SortedArray_u32;
typedef SortedArray(u16) SortedArray_u16;
typedef SortedArray(u08) SortedArray_u08;
typedef SortedArray(s32) SortedArray_s32;
typedef SortedArray(s16) SortedArray_s16;
typedef SortedArray(s08) SortedArray_s08;

#endif
