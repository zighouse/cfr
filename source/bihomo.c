#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

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

    bihomographic * bh = (bihomographic*) c;

    while (--limit)
    {
        int input_x;

        if (bh->e == 0 && bh->f == 0 && bh->g == 0 && bh->h == 0) 
        {
            return LLONG_MAX;
        }

        do {

            if (bh->f == 0)
            {
                input_x = (bh->e == 0);
                break;
            }
            ix  = bh->b / bh->f;

            if (bh->g == 0)
            {
                input_x = (bh->e != 0);
                break;
            }
            iy  = bh->c / bh->g;

            ixy = bh->e ? bh->a / bh->e : LLONG_MAX;
            i0  = bh->h ? bh->d / bh->h : LLONG_MAX;

            if (ixy == ix && ix == iy && iy == i0)
            {
                /* output a term */
                long long a, b, c, d, e, f, g, h;

                if (ixy < 0)
                {
                    --ixy;
                }

                a = bh->a;     b = bh->b;     c = bh->c;     d = bh->d;
                bh->a = bh->e;  bh->b = bh->f;  bh->c = bh->g;  bh->d = bh->h;

                e = a - ixy * bh->e;
                f = b - ixy * bh->f;
                g = c - ixy * bh->g;
                h = d - ixy * bh->h;
                // detect overflow
                if (e < 0)
                {
                    bh->e = 0;
                    bh->f = 0;
                    bh->g = 0;
                    bh->h = 0;
                }
                else
                {
                    bh->e = e;
                    bh->f = f;
                    bh->g = g;
                    bh->h = h;
                }

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

            if (bh->h == 0)
            {
                input_x = diff(ixy, iy) > diff(ixy, ix) ? 1 : 0;
                break;
            }

            if (bh->e == 0)
            {
                input_x = diff(i0, iy) > diff(i0, ix) ? 1 : 0;
                break;
            }

            input_x = max(diff(ixy, iy), diff(i0, ix)) > max(diff(ixy, ix), diff(i0, iy));
        } while (0);

        if (input_x)
        {
            long long p;

            p = cf_next_term(bh->x);
            if (p == LLONG_MAX && cf_is_finished(bh->x))
            {
                bh->c = bh->a;
                bh->d = bh->b;
                bh->g = bh->e;
                bh->h = bh->f;
            }
            else
            {
                long long a = bh->a, b = bh->b, c, d, e = bh->e, f = bh->f, g, h;
                long long A, B, C, D, E, F, G, H;

                A = a * p + bh->c;
                B = b * p + bh->d;
                C = a;
                D = b;

                E = e * p + bh->g;
                F = f * p + bh->h;
                G = e;
                H = f;

                // detect overflow
                if ((A < 0 || E < 0)
                    && !(bh->a < 0 || bh->b < 0 || bh->c < 0 || bh->d < 0 ||
                         bh->e < 0 || bh->f < 0 || bh->g < 0 || bh->h < 0))
                {
                    long long ret = max( bh->e ? bh->a / bh->e : LLONG_MAX ,
                                         max ( bh->f ? bh->b / bh->f : LLONG_MAX,
                                               bh->g ? bh->c / bh->g : LLONG_MAX ) );
                    // pre-output
                    {
                        a = bh->a;     b = bh->b;     c = bh->c;     d = bh->d;
                        bh->a = bh->e;  bh->b = bh->f;  bh->c = bh->g;  bh->d = bh->h;

                        e = a - ret * bh->e;
                        f = b - ret * bh->f;
                        g = c - ret * bh->g;
                        h = d - ret * bh->h;
                        if (e < 0)
                        {
                            bh->e = 0;
                            bh->f = 0;
                            bh->g = 0;
                            bh->h = 0;
                        }
                        else
                        {
                            A = a * p + c;
                            B = b * p + d;
                            C = a;
                            D = b;

                            E = e * p + g;
                            F = f * p + h;
                            G = e;
                            H = f;

                            if (A < 0 || E < 0)
                            {
                                bh->e = 0;
                                bh->f = 0;
                                bh->g = 0;
                                bh->h = 0;
                            }
                            else
                            {
                                bh->a = A;
                                bh->b = B;
                                bh->c = C;
                                bh->d = D;

                                bh->e = E;
                                bh->f = F;
                                bh->g = G;
                                bh->h = H;
                            }
                        }
                    }
                    return ret;
                }
                else
                {
                    bh->a = A;
                    bh->b = B;
                    bh->c = C;
                    bh->d = D;

                    bh->e = E;
                    bh->f = F;
                    bh->g = G;
                    bh->h = H;
                }
            }
        }
        else
        {
            long long p;

            p = cf_next_term(bh->y);
            if (p == LLONG_MAX && cf_is_finished(bh->y))
            {
                bh->b = bh->a;
                bh->d = bh->c;
                bh->f = bh->e;
                bh->h = bh->g;
            }
            else
            {
                long long a = bh->a, b, c = bh->c, d, e = bh->e, f, g = bh->g, h;
                long long A, B, C, D, E, F, G, H;

                A = a * p + bh->b;
                B = a;
                C = c * p + bh->d;
                D = c;

                E = e * p + bh->f;
                F = e;
                G = g * p + bh->h;
                H = g;

                // detect overflow
                if ((A < 0 || E < 0)
                    && !(bh->a < 0 || bh->b < 0 || bh->c < 0 || bh->d < 0 ||
                         bh->e < 0 || bh->f < 0 || bh->g < 0 || bh->h < 0))
                {
                    long long ret = max( bh->e ? bh->a / bh->e : LLONG_MAX ,
                                         max ( bh->f ? bh->b / bh->f : LLONG_MAX,
                                               bh->g ? bh->c / bh->g : LLONG_MAX ) );
                    // pre-output
                    {
                        a = bh->a;     b = bh->b;     c = bh->c;     d = bh->d;
                        bh->a = bh->e;  bh->b = bh->f;  bh->c = bh->g;  bh->d = bh->h;

                        e = a - ret * bh->e;
                        f = b - ret * bh->f;
                        g = c - ret * bh->g;
                        h = d - ret * bh->h;
                        if (e < 0)
                        {
                            bh->e = 0;
                            bh->f = 0;
                            bh->g = 0;
                            bh->h = 0;
                        }
                        else
                        {
                            A = a * p + b;
                            B = a;
                            C = c * p + d;
                            D = c;

                            E = e * p + f;
                            F = e;
                            G = g * p + h;
                            H = g;

                            if (A < 0 || E < 0)
                            {
                                bh->e = 0;
                                bh->f = 0;
                                bh->g = 0;
                                bh->h = 0;
                            }
                            else
                            {
                                bh->a = A;
                                bh->b = B;
                                bh->c = C;
                                bh->d = D;

                                bh->e = E;
                                bh->f = F;
                                bh->g = G;
                                bh->h = H;
                            }
                        }
                    }
                    return ret;
                }
                else
                {
                    bh->a = A;
                    bh->b = B;
                    bh->c = C;
                    bh->d = D;

                    bh->e = E;
                    bh->f = F;
                    bh->g = G;
                    bh->h = H;
                }
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
