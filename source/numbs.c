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

static int numbers_is_finished(const cf * c)
{
    return ((numbers*)c)->idx >= ((numbers*)c)->size;
}

static void numbers_free(cf *c)
{
    free(((numbers*)c)->arr);
    free(c);
}

static cf * numbers_copy(const cf * c)
{
    numbers * n = (numbers*)c;
    return cf_create_from_terms(n->arr + n->idx, n->size - n->idx);
}

static cf_class _numbers_class = {
    numbers_next_term,
    numbers_is_finished,
    numbers_free,
    numbers_copy
};

cf * cf_create_from_terms(const long long * arr, unsigned int size)
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

#include <stdarg.h>
cf * cf_create_from_terms_i(unsigned int number_of_int, ...)
{
    cf * c;
    long long *arr = (long long*)malloc(sizeof(long long) * number_of_int);
    unsigned int i;
    va_list ap;

    if (number_of_int == 0 || number_of_int > 1024)
    {
        return NULL;
    }

    if (!arr)
    {
        return NULL;
    }

    va_start(ap, number_of_int);
    for (i = 0; i < number_of_int; ++i)
    {
        arr[i] = (long long)va_arg(ap, int);
    }
    va_end(ap);

    c = cf_create_from_terms(arr, number_of_int);
    free(arr);

    return c;
}

cf * cf_create_from_terms_ll(unsigned int number_of_longlong, ...)
{
    cf * c;
    long long *arr = (long long*)malloc(sizeof(long long) * number_of_longlong);
    unsigned int i;
    va_list ap;

    if (number_of_longlong == 0 || number_of_longlong > 1024)
    {
        return NULL;
    }

    if (!arr)
    {
        return NULL;
    }

    va_start(ap, number_of_longlong);
    for (i = 0; i < number_of_longlong; ++i)
    {
        arr[i] = va_arg(ap, long long);
    }
    va_end(ap);

    c = cf_create_from_terms(arr, number_of_longlong);
    free(arr);

    return c;
}
