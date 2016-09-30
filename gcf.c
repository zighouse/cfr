#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "cf.h"

static gcf_class _pnumbers_class;

typedef struct _pnumbers pnumbers;
struct _pnumbers {
    gcf base;
    number_pair *arr;
    unsigned int size;
    unsigned int idx;
};

static number_pair pnumbers_next_term(gcf *g)
{
    pnumbers * n = (pnumbers*)g;
    return n->idx < n->size ?
        n->arr[n->idx++] : (number_pair){1ll, LLONG_MAX};
}

static int pnumbers_is_finished(const gcf * const g)
{
    return ((pnumbers*)g)->idx >= ((pnumbers*)g)->size;
}

static void pnumbers_free(gcf *g)
{
    free(((pnumbers*)g)->arr);
    free(g);
}

static gcf * pnumbers_copy(const gcf * const g)
{
    pnumbers * n = (pnumbers*)g;
    return gcf_create_from_pairs(n->arr + n->idx, n->size - n->idx);
}

static gcf_class _pnumbers_class = {
    pnumbers_next_term,
    pnumbers_is_finished,
    pnumbers_free,
    pnumbers_copy
};

gcf * gcf_create_from_pairs(const number_pair * const arr, unsigned int size)
{
    pnumbers * n;

    if (!size || !arr)
        return NULL;

    n = (pnumbers*)malloc(sizeof(pnumbers));
    if (!n)
        return NULL;

    n->arr = (number_pair*)malloc(size * sizeof(number_pair));
    memcpy(n->arr, arr, size * sizeof(number_pair));
    n->idx = 0;
    n->size = size;
    n->base.object_class = &_pnumbers_class;
    return &n->base;
}

static cf_class _ghomo_class;

typedef struct _ghomo ghomo;
struct _ghomo {
    cf base;
    long long a, b, c, d;
    gcf * x;
};

static long long ghomo_next_term(cf *c)
{
    long long i1, i0;
    number_pair p;

    unsigned limit = 10000;

    ghomo * h = (ghomo*) c;

    while (--limit)
    {
        i1 = h->c ? h->a / h->c : LLONG_MAX;
        i0 = h->d ? h->b / h->d : LLONG_MAX;

        if (i1 == i0)
        {
            long long a = h->a, b = h->b;

            if (h->c == 0ll && h->d == 0ll)
                return LLONG_MAX;

            if (i1 < 0)
                --i1;

            h->a = h->c;
            h->b = h->d;

            h->c = a - i1 * h->c;
            h->d = b - i1 * h->d;
            return i1;
        }

        p = cf_next_term(h->x);
        if (p.b == LLONG_MAX && cf_is_finished(h->x))
        {
            h->b = h->a;
            h->d = h->c;
        }
        else
        {
            long long a = h->a, c = h->c;

            h->a = a * p.b + h->b * p.a;
            h->b = a;
            h->c = c * p.b + h->d * p.a;
            h->d = c;
        }
    }
    return LLONG_MAX;
}

static int ghomo_is_finished(const cf * const c)
{
    ghomo * h = (ghomo*) c;
    return h->c == 0ll && h->d == 0ll;
}

static void ghomo_free(cf *c)
{
    ghomo * h = (ghomo*) c;
    cf_free(h->x);
    free(h);
}

static cf * ghomo_copy(const cf * const c)
{
    ghomo * h = (ghomo*) c;
    return cf_create_from_ghomo(h->x, h->a, h->b, h->c, h->d);
}

static cf_class _ghomo_class = {
    ghomo_next_term,
    ghomo_is_finished,
    ghomo_free,
    ghomo_copy
};

cf * cf_create_from_ghomo(const gcf * const x,
                          long long a, long long b,
                          long long c, long long d)
{
    ghomo * h = (ghomo*)malloc(sizeof(ghomo));

    if (!h)
        return NULL;

    h->base.object_class = &_ghomo_class;

    h->a = a;
    h->b = b;
    h->c = c;
    h->d = d;
    h->x = cf_copy(x);
    return &h->base;
}
