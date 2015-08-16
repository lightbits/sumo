#include "sorted_array.h"

// returns true and the index of the matching element
// or false and the insertion index of "key" to preserve
// lexicographic ordering.
bool binary_search(char **keys, int n, char *key, int *result)
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

void sorted_array_alloc_(SortedArrayBase *base,
                         u32 count, u32 element_size)
{
    base->keys = (char**)malloc(count);
    base->data = (u08*)malloc(count * element_size);
    base->size = count;
    base->used = 0;
}

void free_sorted_array_(SortedArrayBase *base)
{
    for (u32 i = 0; i < base->used; i++)
        free(base->keys[i]);
    free(base->keys);
    free(base->data);
    base->size = 0;
    base->used = 0;
}

void sorted_array_set_(SortedArrayBase *s, char *key, u08 *value, u32 size)
{
    assert(s->used < s->size);
    int insert_at;
    if (binary_search(s->keys, s->used, key, &insert_at))
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

void *sorted_array_get_(SortedArrayBase *s, char *key, u32 size)
{
    int i;
    bool exists = binary_search(s->keys, s->used, key, &i);
    if (exists)
        return s->data + size * i;
    else
        return 0;
}
