#include <stdio.h>
#include <stdlib.h>

struct Mesh
{
    float vertices[3*1024];
};

bool soi_is_space(char c)
{
    return c == ' ' || c == '\n';
}

bool soi_is_float(char *begin, char *end)
{
    char *c = begin;
    while (c < end + 1)
    {
        if ((*c < '0' || *c > '9') && *c != '.' && *c != '-')
            return false;
        c++;
    }
    return true;
}

bool soi_is_int(char *begin, char *end)
{
    char *c = begin;
    while (c < end + 1)
    {
        if ((*c < '0' || *c > '9') & *c != '-')
            return false;
        c++;
    }
    return true;
}

char *soi_read_space_seperated_string(char *s, char **begin, char **end)
{
    while (*s && soi_is_space(*s)) s++;
    *begin = s;
    while (*s && !soi_is_space(*s)) s++;
    *end = s - 1;
    return s;
}

void print_range(char *begin, char *end)
{
    printf("'");
    while (begin < end + 1)
    {
        printf("%c", *begin);
        begin++;
    }
    printf("'");
}

bool soi_is_comment(char *c)
{
    return *c == '#';
}

bool soi_cmp(char *a, char *b)
{
    while (*a && *b)
    {
        if (*a != *b)
            return false;
        a++;
        b++;
    }
    return true;
}

Mesh so_load_obj_from_memory(char *data, long len)
{
    Mesh result = {};
    char *read_ptr = data;
    while (*read_ptr && (read_ptr - data) <= len)
    {
        char *begin;
        char *end;
        read_ptr = soi_read_space_seperated_string(read_ptr, &begin, &end);
        if (begin > end)
            break;

        if (soi_is_comment(begin))
        {
            printf("comment\n");
            while (*end && *end != '\n')
                end++;
            read_ptr = end + 1;
        }
        else if (soi_cmp(begin, "mtllib"))
        {
            read_ptr = soi_read_space_seperated_string(read_ptr, &begin, &end);

            printf("mtllib: ");
            print_range(begin, end);
            printf("\n");
        }
        else if (soi_cmp(begin, "usemtl"))
        {
            read_ptr = soi_read_space_seperated_string(read_ptr, &begin, &end);
            printf("usemtl: ");
            print_range(begin, end);
            printf("\n");
        }
        else if (*begin == 'o')
        {
            printf("shape: ");
            read_ptr = soi_read_space_seperated_string(read_ptr, &begin, &end);
            print_range(begin, end);
            printf("\n");
        }
        else if (*begin == 'v')
        {
            printf("vertex: ");
            float v[4];
            int i = 0;
            read_ptr = soi_read_space_seperated_string(read_ptr, &begin, &end);
            while (soi_is_float(begin, end))
            {
                v[i] = (float)atof(begin);
                printf("%.5f ", v[i]);
                read_ptr = soi_read_space_seperated_string(read_ptr, &begin, &end);
                i++;
            }
            read_ptr = begin - 1;
            printf("\n");
        }
        else if (*begin == 'f')
        {
            printf("face: ");
            int f[4];
            int i = 0;
            read_ptr = soi_read_space_seperated_string(read_ptr, &begin, &end);
            while (*read_ptr && soi_is_int(begin, end))
            {
                f[i] = atoi(begin);
                printf("%d ", f[i]);
                read_ptr = soi_read_space_seperated_string(read_ptr, &begin, &end);
                i++;
            }
            read_ptr = begin - 1;
            printf("\n");
        }
    }
    return result;
}

Mesh so_load_obj(char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (!f)
    {
        printf("Failed to load obj: Could not open file %s\n", filename);
        exit(-1);
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    char *data = new char[size + 1];
    if (!data)
    {
        printf("Failed to load obj: File too large %s\n", filename);
        exit(-1);
    }
    if (!fread(data, 1, size, f))
    {
        printf("Failed to load obj: Could not read file %s\n", filename);
        exit(-1);
    }
    fclose(f);
    data[size] = 0;
    return so_load_obj_from_memory(data, size);
}

int main(int argc, char **argv)
{
    so_load_obj("./assets/teapot.obj");
    return 0;
}
