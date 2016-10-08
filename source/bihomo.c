#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "cf.h"

static cf_class _bihomographic_class;

typedef struct _bihomographic bihomographic;
struct _bihomographic {
    cf base;
    long long a, b, c, d, e, f, g, h;
    cf * x, * y;
};

static long long diff(long long x, long long y)
{
    return x > y ? x - y : y - x;
}

static long long max(long long x, long long y)
{
    return x > y ? x : y;
}

static long long bihomographic_next_term(cf *c)
{
    long long ixy, ix, iy, i0;
    unsigned limit = 10000;

    bihomographic * h = (bihomographic*) c;

    while (--limit)
    {
        int input_x;

        if (h->e == 0 && h->f == 0 && h->g == 0 && h->h == 0) 
        {
            return LLONG_MAX;
        }

        do {

            if (h->f == 0)
            {
                input_x = 0;
                break;
            }
            ix  = h->b / h->f;

            if (h->g == 0)
            {
                input_x = 1;
                break;
            }
            iy  = h->c / h->g;

            ixy = h->e ? h->a / h->e : LLONG_MAX;
            i0  = h->h ? h->d / h->h : LLONG_MAX;

            if (ixy == ix && ix == iy && iy == i0)
            {
                /* output a term */
                long long a, b, c, d;

                if (ixy < 0)
                {
                    --ixy;
                }

                a = h->a;     b = h->b;     c = h->c;     d = h->d;
                h->a = h->e;  h->b = h->f;  h->c = h->g;  h->d = h->h;

                h->e = a - ixy * h->e;
                h->f = b - ixy * h->f;
                h->g = c - ixy * h->g;
                h->h = d - ixy * h->h;
                return ixy;
            }

            if (ixy == ix && ix != 0)
            {
                input_x = 1;
                break;
            }

            if (ixy == iy && iy != 0)
            {
                input_x = 0;
                break;
            }

            if (h->h == 0)
            {
                input_x = diff(ixy, iy) > diff(ixy, ix) ? 1 : 0;
                break;
            }

            if (h->e == 0)
            {
                input_x = diff(i0, iy) > diff(i0, ix) ? 1 : 0;
                break;
            }

            input_x = max(diff(ixy, iy), diff(i0, ix)) > max(diff(ixy, ix), diff(i0, iy));
        } while (0);

        if (input_x)
        {
            long long p;

            p = cf_next_term(h->x);
            if (p == LLONG_MAX && cf_is_finished(h->x))
            {
                h->c = h->a;
                h->d = h->b;
                h->g = h->e;
                h->h = h->f;
            }
            else
            {
                long long a = h->a, b = h->b, e = h->e, f = h->f;

                h->a = a * p + h->c;
                h->b = b * p + h->d;
                h->c = a;
                h->d = b;

                h->e = e * p + h->g;
                h->f = f * p + h->h;
                h->g = e;
                h->h = f;
            }
        }
        else
        {
            long long p;

            p = cf_next_term(h->y);
            if (p == LLONG_MAX && cf_is_finished(h->y))
            {
                h->b = h->a;
                h->d = h->c;
                h->f = h->e;
                h->h = h->g;
            }
            else
            {
                long long a = h->a, c = h->c, e = h->e, g = h->g;

                h->a = a*p + h->b;
                h->b = a;
                h->c = c*p + h->d;
                h->d = c;

                h->e = e*p + h->f;
                h->f = e;
                h->g = g*p + h->h;
                h->h = g;
            }
        }
    }
    return LLONG_MAX;
}

static int bihomographic_is_finished(const cf * c)
{
    bihomographic * h = (bihomographic*) c;
    return h->e == 0ll && h->f == 0ll && h->g == 0ll && h->h == 0ll;
}

static void bihomographic_free(cf *c)
{
    bihomographic * h = (bihomographic*) c;
    cf_free(h->x);
    cf_free(h->y);
    free(h);
}

static cf * bihomographic_copy(const cf * c)
{
    bihomographic * h = (bihomographic*) c;
    return cf_create_from_bihomographic(h->x, h->y,
                                        h->a, h->b, h->c, h->d,
                                        h->e, h->f, h->g, h->h);
}

static cf_class _bihomographic_class = {
    bihomographic_next_term,
    bihomographic_is_finished,
    bihomographic_free,
    bihomographic_copy
};

cf * cf_create_from_bihomographic(const cf * x, const cf * y,
                                  long long a, long long b, long long c, long long d,
                                  long long e, long long f, long long g, long long h)
{
    bihomographic * bh = (bihomographic*)malloc(sizeof(bihomographic));

    if (!bh)
        return NULL;

    bh->base.object_class = &_bihomographic_class;

    bh->a = a; bh->b = b; bh->c = c; bh->d = d;
    bh->e = e; bh->f = f; bh->g = g; bh->h = h;
    bh->x = cf_copy(x);
    bh->y = cf_copy(y);
    return &bh->base;
}
