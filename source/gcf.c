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
 * one algorithm to calculate sqrt(n) (not the best one)
 *
 *                     n - 1
 * sqrt(n) = 1 + --------------------
 *                        n - 1
 *               2 + ----------------
 *                          n - 1
 *                   2 + ------------
 *                            n - 1
 *                       2 + --------
 *                           2 + ...
 *        = gcf({1,1},{n-1,2},{n-1,2},{n-1,2}...)
 */
typedef struct _gcf_sqrt_n gcf_sqrt_n;
static gcf_class _gcf_sqrt_n_class;
struct _gcf_sqrt_n {
    gcf base;
    unsigned long long n_minus_one;
    int got_first;
};

static number_pair gcf_sqrt_n_next_term(gcf *g)
{
    gcf_sqrt_n * sqrt_n = (gcf_sqrt_n*)g;
    if (sqrt_n->got_first)
    {
        return (number_pair){sqrt_n->n_minus_one, 2ll};
    }
    else
    {
        sqrt_n->got_first = 1;
        return (number_pair){1, 1};
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
    return gcf_create_from_sqrt_n(sqrt_n->n_minus_one + 1ll);
}

static gcf_class _gcf_sqrt_n_class = {
    gcf_sqrt_n_next_term,
    gcf_sqrt_n_is_finished,
    gcf_sqrt_n_free,
    gcf_sqrt_n_copy
};

/*
 * If x is square, return +sqrt(x).
 * Otherwise, return -1.
 * https://www.quora.com/What-is-the-quickest-way-to-determine-if-a-number-is-a-perfect-square
 */
static
#if 0
long long isqrt(unsigned long long x)
{
    /* Precondition: make x odd if possible. */
    int sh = __builtin_ctzll(x);
    x >>= (sh&~1);

    /* Early escape. */
    if (x&6) return -1;

    /* 2-adic Newton */
    int i;
    const int ITERATIONS = 5; /* log log x - 1; log LLONG_MAX < 64; log 64 < 5 */
    unsigned long long z = (3-x)>>1, y=x*z;
    for (i=0; i<ITERATIONS; i++) {
        unsigned long long w = (3 - z*y) >> 1;
        y *= w;
        z *= w;
    }
    /* assert(x==0 || (y*z == 1 && x*z == y)); */

    /* Get the positive square root.  Also the top bit might be wrong. */
    if (y & (1ull<<62)) y = -y;
    y &= ~(1ull<<63);

    /* Is it the square root? */
    if (y >= 1ull<<32) return -1;

    /* Yup. */
    return y<<(sh/2);
}
#else
long long isqrt(unsigned long long x)
{
    unsigned long long y = sqrt(x);
    if (y*y == x) return y;
    return -1;
}
#endif

gcf * gcf_create_from_sqrt_n(unsigned long long n)
{
    long long y = isqrt(n);
    if (y != -1)
    {
        number_pair p[] = {{1ll, y},{1ll, LLONG_MAX}};
        return gcf_create_from_pairs(p, 2);
    }
    else
    {
        gcf_sqrt_n * sqrt_n = (gcf_sqrt_n*)malloc(sizeof(gcf_sqrt_n));
        if (!sqrt_n)
            return NULL;
        sqrt_n->n_minus_one = n - 1;
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
