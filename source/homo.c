#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "cf.h"

static cf_class _homographic_class;

typedef struct _homographic homographic;
struct _homographic {
    cf base;
    long long a, b, c, d;
    cf * x;
};

static long long homographic_next_term(cf *c)
{
    long long i1, i0;
    long long p;

    unsigned limit = 10000;

    homographic * h = (homographic*) c;

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
        if (p == LLONG_MAX && cf_is_finished(h->x))
        {
            h->b = h->a;
            h->d = h->c;
        }
        else
        {
            long long a = h->a, c = h->c;

            h->a = a * p + h->b;
            h->b = a;
            h->c = c * p + h->d;
            h->d = c;
        }
    }
    return LLONG_MAX;
}

static int homographic_is_finished(const cf * c)
{
    homographic * h = (homographic*) c;
    return h->c == 0ll && h->d == 0ll;
}

static void homographic_free(cf *c)
{
    homographic * h = (homographic*) c;
    cf_free(h->x);
    free(h);
}

static cf * homographic_copy(const cf * c)
{
    homographic * h = (homographic*) c;
    return cf_create_from_homographic(h->x, h->a, h->b, h->c, h->d);
}

static cf_class _homographic_class = {
    homographic_next_term,
    homographic_is_finished,
    homographic_free,
    homographic_copy
};

cf * cf_create_from_homographic(const cf * x,
                               long long a, long long b,
                               long long c, long long d)
{
    homographic * h = (homographic*)malloc(sizeof(homographic));

    if (!h)
        return NULL;

    h->base.object_class = &_homographic_class;

    h->a = a;
    h->b = b;
    h->c = c;
    h->d = d;
    h->x = cf_copy(x);
    return &h->base;
}
