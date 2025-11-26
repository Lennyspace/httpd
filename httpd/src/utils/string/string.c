#include "string.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct string *string_create(const char *str, size_t size)
{
    struct string *new = malloc(sizeof(struct string));
    new->data = malloc(size);
    memcpy(new->data, str, size);
    new->size = size;
    return new;
}

int string_compare_n_str(const struct string *str1, const char *str2, size_t n)
{
    if (!str1 || !str1->data)
    {
        return -1;
    }
    if (str1->size < n)
    {
        return -1;
    }
    return memcmp(str1->data, str2, n);
}

void string_concat_str(struct string *str, const char *to_concat, size_t size)
{
    str->data = realloc(str->data, str->size + size);
    for (size_t i = 0; i < size; i++)
    {
        str->data[str->size + i] = to_concat[i];
    }
    str->size = str->size + size;
}

void string_destroy(struct string *str)
{
    free(str->data);
    free(str);
}
