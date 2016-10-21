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

/* macro detect_overflow(is_overflow) */
#if defined(__GNUC__) && defined(i386)
#define detect_overflow(is_overflow)                         \
{                                                            \
    size_t eflags;                                           \
	__asm__ (                                                \
		"pushfl;"                                            \
		"popl %%eax"                                         \
      : "=a" (eflags));                                      \
    is_overflow |= (eflags >> 11) & 1;                       \
}
#elif defined(__GNUC__) && defined(__x86_64__)
#define detect_overflow(is_overflow)                         \
{                                                            \
    size_t rflags;                                           \
	__asm__ (                                                \
		"pushfq;"                                            \
		"popq %%rax"                                         \
      : "=a" (rflags));                                      \
    is_overflow |= (rflags >> 11) & 1;                       \
}
#else
#error detect_overflow() not implemented!
#endif

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
                int is_overflow = 0;

                if (ixy < 0)
                {
                    --ixy;
                }

                    a = bh->a;      b = bh->b;      c = bh->c;      d = bh->d;
                bh->a = bh->e;  bh->b = bh->f;  bh->c = bh->g;  bh->d = bh->h;

                e = ixy * bh->e;     detect_overflow(is_overflow);
                f = ixy * bh->f;     detect_overflow(is_overflow);
                g = ixy * bh->g;     detect_overflow(is_overflow);
                h = ixy * bh->h;     detect_overflow(is_overflow);

                if (is_overflow)
                {
                    bh->e = 0;
                    bh->f = 0;
                    bh->g = 0;
                    bh->h = 0;
                }
                else
                {
                    bh->e = a - e;
                    bh->f = b - f;
                    bh->g = c - g;
                    bh->h = d - h;
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
                long long t1, t2, t3, t4;
                int is_overflow = 0;

                t1 = a * p;         detect_overflow(is_overflow);
                t2 = b * p;         detect_overflow(is_overflow);
                t3 = e * p;         detect_overflow(is_overflow);
                t4 = f * p;         detect_overflow(is_overflow);
                
                A = t1 + bh->c;     detect_overflow(is_overflow);
                B = t2 + bh->d;     detect_overflow(is_overflow);
                C = a;
                D = b;

                E = t3 + bh->g;     detect_overflow(is_overflow);
                F = t4 + bh->h;     detect_overflow(is_overflow);
                G = e;
                H = f;

                if (is_overflow) /* {{{ handle overflow exception */
                {
                    long long ret = max( bh->e ? bh->a / bh->e : LLONG_MAX ,
                                         max ( bh->f ? bh->b / bh->f : LLONG_MAX,
                                               bh->g ? bh->c / bh->g : LLONG_MAX ) );
                    // pre-output
                    {
                            a = bh->a;      b = bh->b;      c = bh->c;      d = bh->d;
                        bh->a = bh->e;  bh->b = bh->f;  bh->c = bh->g;  bh->d = bh->h;

                        is_overflow = 0;

                        t1 = ret * bh->e;   detect_overflow(is_overflow);
                        t2 = ret * bh->f;   detect_overflow(is_overflow);
                        t3 = ret * bh->g;   detect_overflow(is_overflow);
                        t4 = ret * bh->h;   detect_overflow(is_overflow);

                        e = a - t1;
                        f = b - t2;
                        g = c - t3;
                        h = d - t4;

                        if (is_overflow)
                        {
                            bh->e = 0;
                            bh->f = 0;
                            bh->g = 0;
                            bh->h = 0;
                        }
                        else
                        {
                            is_overflow = 0;

                            t1 = a * p;       detect_overflow(is_overflow);
                            t2 = b * p;       detect_overflow(is_overflow);
                            t3 = e * p;       detect_overflow(is_overflow);
                            t4 = f * p;       detect_overflow(is_overflow);

                            A = t1 + c;       detect_overflow(is_overflow);
                            B = t2 + d;       detect_overflow(is_overflow);
                            C = a;
                            D = b;

                            E = t3 + g;       detect_overflow(is_overflow);
                            F = t4 + h;       detect_overflow(is_overflow);
                            G = e;
                            H = f;

                            if (is_overflow)
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
                } /* overflow exception handled }}} */
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
                long long t1, t2, t3, t4;
                int is_overflow = 0;

                t1 = a * p;        detect_overflow(is_overflow);
                t2 = c * p;        detect_overflow(is_overflow);
                t3 = e * p;        detect_overflow(is_overflow);
                t4 = g * p;        detect_overflow(is_overflow);

                A = t1 + bh->b;    detect_overflow(is_overflow);
                B = a;
                C = t2 + bh->d;    detect_overflow(is_overflow);
                D = c;

                E = t3 + bh->f;    detect_overflow(is_overflow);
                F = e;
                G = t4 + bh->h;    detect_overflow(is_overflow);
                H = g;

                if (is_overflow)  /* {{{ handle overflow exception */
                {
                    long long ret = max( bh->e ? bh->a / bh->e : LLONG_MAX ,
                                         max ( bh->f ? bh->b / bh->f : LLONG_MAX,
                                               bh->g ? bh->c / bh->g : LLONG_MAX ) );
                    // pre-output
                    {
                            a = bh->a;      b = bh->b;      c = bh->c;      d = bh->d;
                        bh->a = bh->e;  bh->b = bh->f;  bh->c = bh->g;  bh->d = bh->h;

                        is_overflow = 0;

                        t1 = ret * bh->e;      detect_overflow(is_overflow);
                        t2 = ret * bh->f;      detect_overflow(is_overflow);
                        t3 = ret * bh->g;      detect_overflow(is_overflow);
                        t4 = ret * bh->h;      detect_overflow(is_overflow);

                        e = a - t1;
                        f = b - t2;
                        g = c - t3;
                        h = d - t4;

                        if (is_overflow)
                        {
                            bh->e = 0;
                            bh->f = 0;
                            bh->g = 0;
                            bh->h = 0;
                        }
                        else
                        {
                            is_overflow = 0;

                            t1 = a * p;      detect_overflow(is_overflow);
                            t2 = c * p;      detect_overflow(is_overflow);
                            t3 = e * p;      detect_overflow(is_overflow);
                            t4 = g * p;      detect_overflow(is_overflow);

                            A = t1 + b;      detect_overflow(is_overflow);
                            B = a;
                            C = t2 + d;      detect_overflow(is_overflow);
                            D = c;

                            E = t3 + f;      detect_overflow(is_overflow);
                            F = e;
                            G = t4 + h;      detect_overflow(is_overflow);
                            H = g;

                            if (is_overflow)
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
                } /* overflow exception is handled }}} */
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

/* vim:set fdm=marker: */
