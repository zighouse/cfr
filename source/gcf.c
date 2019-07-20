#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>

#include "cf.h"
#include "common.h"

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

static int pnumbers_is_finished(const gcf * g)
{
    return ((pnumbers*)g)->idx >= ((pnumbers*)g)->size;
}

static void pnumbers_free(gcf *g)
{
    free(((pnumbers*)g)->arr);
    free(g);
}

static gcf * pnumbers_copy(const gcf * g)
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

gcf * gcf_create_from_pairs(const number_pair * arr, unsigned int size)
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
    mpz_t a, b, c, d;
    gcf * x;
};

static long long ghomo_next_term(cf *g)
{
    unsigned int limit = UINT_MAX;
    mpz_t i0, i1, a, b, c, t1, t2, t3, t4;
    long long result = LLONG_MAX;
    number_pair p;
    ghomo * h = (ghomo*) g;

    mpz_inits(i0, i1, a, b, c, t1, t2, t3, t4, NULL);

    while (--limit)
    {
        if (mpz_sgn(h->c) != 0)
        {
            mpz_fdiv_q(i1, h->a, h->c);
        }
        else
        {
            mpz_set_ll(i1, LLONG_MAX);
        }
        if (mpz_sgn(h->d) != 0)
        {
            mpz_fdiv_q(i0, h->b, h->d);
        }
        else
        {
            mpz_set_ll(i0, LLONG_MAX);
        }

        if (mpz_cmp(i1, i0) == 0)
        {
            mpz_set(a, h->a);
            mpz_set(b, h->b);

            if (mpz_sgn(h->c) == 0 && mpz_sgn(h->d) == 0)
            {
                result = LLONG_MAX;
                goto EXIT_FUNC;
            }

            //if (mpz_sgn(i1) < 0)
            //{
            //    mpz_sub_ui(i1, i1, 1);
            //}

            mpz_set(h->a, h->c);
            mpz_set(h->b, h->d);

            mpz_mul(t1, i1, h->c);
            mpz_mul(t2, i1, h->d);
            mpz_sub(h->c, a, t1);
            mpz_sub(h->d, b, t2);
            result = mpz_get_ll(i1);
            goto EXIT_FUNC;
        }

        p = cf_next_term(h->x);
        if (p.b == LLONG_MAX && cf_is_finished(h->x))
        {
            mpz_set(h->b, h->a);
            mpz_set(h->d, h->c);
        }
        else
        {
            mpz_set(a, h->a);
            mpz_set(c, h->c);
            mpz_set_ll(t3, p.a);
            mpz_set_ll(t4, p.b);

            mpz_mul(t1, h->a, t4);
            mpz_mul(t2, h->b, t3);
            mpz_add(h->a, t1, t2);

            mpz_set(h->b, a);

            mpz_mul(t1, c, t4);
            mpz_mul(t2, h->d, t3);
            mpz_add(h->c, t1, t2);

            mpz_set(h->d, c);
        }
    }
EXIT_FUNC:
    mpz_clears(i0, i1, a, b, c, t1, t2, t3, t4, NULL);
    return result;
}

static int ghomo_is_finished(const cf * c)
{
    ghomo * h = (ghomo*) c;
    return mpz_sgn(h->c) == 0 && mpz_sgn(h->d) == 0;
}

static void ghomo_free(cf *c)
{
    ghomo * h = (ghomo*) c;
    mpz_clears(h->a, h->b, h->c, h->d, NULL);
    cf_free(h->x);
    free(h);
}

static
cf * cf_create_from_ghomo_mpz(const gcf * x,
                              mpz_t a, mpz_t b,
                              mpz_t c, mpz_t d);
static cf * ghomo_copy(const cf * c)
{
    ghomo * h = (ghomo*) c;
    return cf_create_from_ghomo_mpz(h->x, h->a, h->b, h->c, h->d);
}

static cf_class _ghomo_class = {
    ghomo_next_term,
    ghomo_is_finished,
    ghomo_free,
    ghomo_copy
};

static
cf * cf_create_from_ghomo_mpz(const gcf * x,
                              mpz_t a, mpz_t b,
                              mpz_t c, mpz_t d)
{
    ghomo * h = (ghomo*)malloc(sizeof(ghomo));

    if (!h)
        return NULL;

    h->base.object_class = &_ghomo_class;

    mpz_inits(h->a, h->b, h->c, h->d, NULL);
    mpz_set(h->a, a);
    mpz_set(h->b, b);
    mpz_set(h->c, c);
    mpz_set(h->d, d);
    h->x = cf_copy(x);
    return &h->base;
}

cf * cf_create_from_ghomo(const gcf * x,
                          long long a, long long b,
                          long long c, long long d)
{
    ghomo * h = (ghomo*)malloc(sizeof(ghomo));

    if (!h)
        return NULL;

    h->base.object_class = &_ghomo_class;

    mpz_inits(h->a, h->b, h->c, h->d, NULL);
    mpz_set_ll(h->a, a);
    mpz_set_ll(h->b, b);
    mpz_set_ll(h->c, c);
    mpz_set_ll(h->d, d);
    h->x = cf_copy(x);
    return &h->base;
}

/*
 * one algorithm to calculate pi (not the best one)
 *
 *                    4
 * pi = 0 + ----------------------
 *                      1
 *          1 + ------------------
 *                         4
 *               3 + -------------
 *                           9
 *                   5 + ---------
 *                             16
 *                       7 + -----
 *                            ...
 *
 * http://www.virtuescience.com/pi-in-other-bases.html
 */
typedef struct _gcf_pi gcf_pi;
static gcf_class _gcf_pi_class;
struct _gcf_pi {
    gcf base;
    long long idx;
};

static number_pair gcf_pi_next_term(gcf *g)
{
    long long idx;
    gcf_pi * pi = (gcf_pi*)g;
    idx = pi->idx ++;
    if (idx == 0)
    {
        return (number_pair){1ll, 0ll};
    }
    else if (idx == 1)
    {
        return (number_pair){4ll, 1ll};
    }
    else
    {
        return (number_pair){(idx-1)*(idx-1),idx*2-1ll};
    }
}

static int gcf_pi_is_finished(const gcf * g)
{
    return 0;
}

static void gcf_pi_free(gcf *g)
{
    free(g);
}

static gcf * gcf_pi_copy(const gcf * g)
{
    return gcf_create_from_pi();
}

static gcf_class _gcf_pi_class = {
    gcf_pi_next_term,
    gcf_pi_is_finished,
    gcf_pi_free,
    gcf_pi_copy
};

gcf * gcf_create_from_pi(void)
{
    gcf_pi * pi = (gcf_pi*)malloc(sizeof(gcf_pi));
    if (!pi)
        return NULL;
    pi->idx = 0;
    pi->base.object_class = &_gcf_pi_class;
    return &pi->base;
}

cf * cf_create_from_pi(void)
{
    gcf *g = gcf_create_from_pi();
    cf * c = cf_create_from_ghomo(g, 1, 0, 0, 1);
    cf_free(g);
    return c;
}

/*
 * one algorithm to calculate sqrt(n)
 * let m*m is the max square less then or equal to n,
 *
 *                    n - m*m
 * sqrt(n) = m + -------------------
 *                       n - m*m
 *               2m + --------------
 *                          n - m*m
 *                    2m + ---------
 *                         2m + ...
 *        = gcf({1,m},{n-m*m,2m},{n-m*m,2m},...)
 */
typedef struct _gcf_sqrt_n gcf_sqrt_n;
static gcf_class _gcf_sqrt_n_class;
struct _gcf_sqrt_n {
    gcf base;
    unsigned long long m;
    unsigned long long n_minus_mm;
    int got_first;
};

static number_pair gcf_sqrt_n_next_term(gcf *g)
{
    gcf_sqrt_n * sqrt_n = (gcf_sqrt_n*)g;
    if (sqrt_n->got_first)
    {
        return (number_pair){sqrt_n->n_minus_mm, 2 * sqrt_n->m};
    }
    else
    {
        sqrt_n->got_first = 1;
        return (number_pair){1, sqrt_n->m};
    }
}

static int gcf_sqrt_n_is_finished(const gcf *g)
{
    return 0;
}

static void gcf_sqrt_n_free(gcf *g)
{
    free(g);
}

static gcf * gcf_sqrt_n_copy(const gcf *g)
{
    gcf_sqrt_n * sqrt_n = (gcf_sqrt_n*)g;
    return gcf_create_from_sqrt_n(sqrt_n->n_minus_mm + sqrt_n->m * sqrt_n->m);
}

static gcf_class _gcf_sqrt_n_class = {
    gcf_sqrt_n_next_term,
    gcf_sqrt_n_is_finished,
    gcf_sqrt_n_free,
    gcf_sqrt_n_copy
};

gcf * gcf_create_from_sqrt_n(unsigned long long n)
{
    unsigned long long m = (unsigned long long)sqrt(n);
    unsigned long long n_minus_mm = n - m * m;
    if (n_minus_mm == 0)
    {
        number_pair p[] = {{1ll, m},{1ll, LLONG_MAX}};
        return gcf_create_from_pairs(p, 2);
    }
    else
    {
        gcf_sqrt_n * sqrt_n = (gcf_sqrt_n*)malloc(sizeof(gcf_sqrt_n));
        if (!sqrt_n)
            return NULL;
        sqrt_n->m = m;
        sqrt_n->n_minus_mm = n_minus_mm;
        sqrt_n->got_first = 0;
        sqrt_n->base.object_class = &_gcf_sqrt_n_class;
        return &sqrt_n->base;
    }
}

cf * cf_create_from_sqrt_n(unsigned long long n)
{
    gcf * g = gcf_create_from_sqrt_n(n);
    cf * c = cf_create_from_ghomo(g, 1, 0, 0, 1);
    cf_free(g);
    return c;
}

/*
 * Create a GCF which the value is n-th root of v.
 *
 * v^{m/n} = (a^n + b)^{m/n}
 *                                       mb
 *         = a^m + ----------------------------------------------------
 *                                          (n-m)b
 *                 na^{n-m} + -----------------------------------------
 *                                              (n+m)b
 *                            2a^m + ----------------------------------
 *                                                     (2n-m)b
 *                                   3na^{n-m} + ----------------------
 *                                                          (2n+m)b
 *                                               2a^m + ---------------
 *                                                      5na^{n-m} + ...
 *
 * (integers: 0 < m < n)
 *
 * http://myreckonings.com/Dead_Reckoning/Online/Materials/General%20Method%20for%20Extracting%20Roots.pdf
 */
typedef struct _gcf_nth gcf_nth;
static gcf_class _gcf_nth_class;
struct _gcf_nth {
    gcf base;
    unsigned long long am2; /* 2*a^m */
    unsigned long long anmn; /* na^{n-m} */
    unsigned long long bn; /* nb */
    unsigned long long bm; /* mb */
    unsigned long n;
    unsigned long m;
    unsigned long idx; /* term index */
};

static number_pair gcf_nth_next_term(gcf *g)
{
    gcf_nth * nth = (gcf_nth*)g;
    number_pair pair = {0};
    if (nth->idx == 0)
    {
        pair.a = 1;
        pair.b = nth->am2/2;
    }
    else if (nth->idx == 1)
    {
        pair.a = nth->bm;
        pair.b = nth->anmn;
    }
    else if (nth->idx % 2 == 0)
    {
        pair.a = (nth->idx / 2) * nth->bn - nth->bm;
        pair.b = nth->am2;
    }
    else
    {
        pair.a = (nth->idx / 2) * nth->bn + nth->bm;
        pair.b = nth->idx * nth->anmn;
    }
    nth->idx++;
    return pair;
}

static int gcf_nth_is_finished(const gcf *g)
{
    return 0;
}

static void gcf_nth_free(gcf *g)
{
    free(g);
}

static gcf * gcf_nth_copy(const gcf *g)
{
    gcf_nth* nth = (gcf_nth*)g;
    gcf_nth* nth_new = (gcf_nth*)malloc(sizeof(gcf_nth));
    memcpy(nth_new, nth, sizeof(gcf_nth));
    nth_new->idx = 0;
    return &nth_new->base;
}

static gcf_class _gcf_nth_class = {
    gcf_nth_next_term,
    gcf_nth_is_finished,
    gcf_nth_free,
    gcf_nth_copy
};

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

/**
 * find a: a^n <= v; (a+1)^n > v;
 */
static int find_power(unsigned long long v, unsigned long n,
                      unsigned long long *a, unsigned long long *b)
{
    unsigned long long lo = 1;
    unsigned long long hi = v;
    unsigned long long md = (1 + v)/2;
    unsigned long long pow_n_md = md;
    int is_overflow = 0;
    while (1)
    {
        unsigned long m = n;
        while (m > 1)
        {
            pow_n_md *= md;
            // TODO not enough bits.
            detect_overflow(is_overflow);
            if (is_overflow || pow_n_md > v)
            {
                break;
            }
            m--;
        }
        if (is_overflow || v < pow_n_md)
        {
            hi = md;
            md = (lo + md)/2;
            pow_n_md = md;
            if (hi == lo + 1)
            {
                /* found */
                *a = lo;
                unsigned long m = n;
                while (m > 1)
                {
                    pow_n_md *= md;
                    m--;
                }
                *b = v - pow_n_md;
                return 0;
            }
        }
        else if (v == pow_n_md)
        {
            /* found */
            *a = md;
            *b = 0;
            return 1;
        }
        else if (v > pow_n_md)
        {
            lo = md;
            md = (md + hi)/2;
            if (hi == lo + 1)
            {
                /* found */
                *a = lo;
                *b = v - pow_n_md;
                return 0;
            }
            pow_n_md = md;
        }
    }
    *a = *b = 0;
    return 0;
}

gcf * gcf_create_from_nth_root(unsigned long long v, unsigned long n, unsigned long m)
{
    unsigned long long gcd;
    if (m > n)
    {
        /* TODO calculate power, not root */
        return NULL;
    }

    gcd = cf_get_gcd(n, m);
    if (gcd > 1)
    {
        n /= gcd;
        m /= gcd;
    }

    if (n == 1 || v == 1)
    {
        number_pair p[] = {{1ll, v}, {1ll, LLONG_MAX}};
        return gcf_create_from_pairs(p, 2);
    }
    else if (n == 2)
    {
        return gcf_create_from_sqrt_n(v);
    }
    else
    {
        unsigned long long a, b;
        if (find_power(v, n, &a, &b))
        {
            number_pair p[] = {{1ll, a}, {1ll, LLONG_MAX}};
            return gcf_create_from_pairs(p, 2);
        }
        else
        {
            unsigned long i;
            unsigned long end1 = n-m;
            unsigned long end2 = m;
            unsigned long end = end1 > end2 ? end1 : end2;
            unsigned long long aa = 1;
            gcf_nth * nth = (gcf_nth*)malloc(sizeof(gcf_nth));
            if (!nth)
                return NULL;
            for (i = 1; i <= end; i++)
            {
                aa *= a;
                if (i == end1)
                {
                    nth->anmn = aa * n;
                }
                if (i == end2)
                {
                    nth->am2 = aa * 2;
                }
            }
            nth->bn = b * n;
            nth->bm = b * m;
            nth->n = n;
            nth->m = m;
            nth->idx = 0;
            nth->base.object_class = &_gcf_nth_class;

            return &nth->base;
        }
    }
}

cf * cf_create_from_nth_root(unsigned long long v, unsigned long n, unsigned long m)
{
    gcf * g = gcf_create_from_nth_root(v, n, m);
    cf * c;
    if (!g)
    {
        return NULL;
    }
    c = cf_create_from_ghomo(g, 1, 0, 0, 1);
    cf_free(g);
    return c;
}

