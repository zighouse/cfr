#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "cf.h"

static cf_class _numbers_class;

typedef struct _numbers numbers;
struct _numbers {
    cf base;
    long long *arr;
    unsigned int size;
    unsigned int idx;
};

static long long numbers_next_term(cf *c)
{
    numbers * n = (numbers*)c;
    return n->idx < n->size ? n->arr[n->idx++] : LLONG_MAX;
}

static int numbers_is_finished(const cf * const c)
{
    return ((numbers*)c)->idx >= ((numbers*)c)->size;
}

static void numbers_free(cf *c)
{
    free(((numbers*)c)->arr);
    free(c);
}

static cf * numbers_copy(const cf * const c)
{
    numbers * n = (numbers*)c;
    return cf_create_from_numbers(n->arr + n->idx, n->size - n->idx);
}

static cf_class _numbers_class = {
    numbers_next_term,
    numbers_is_finished,
    numbers_free,
    numbers_copy
};

cf * cf_create_from_numbers(const long long * const arr, unsigned int size)
{
    numbers * n;

    if (!size || !arr)
        return NULL;

    n = (numbers*)malloc(sizeof(numbers));
    if (!n)
        return NULL;

    n->arr = (long long*)malloc(size * sizeof(long long));
    memcpy(n->arr, arr, size * sizeof(long long));
    n->idx = 0;
    n->size = size;
    n->base.object_class = &_numbers_class;
    return &n->base;
}
