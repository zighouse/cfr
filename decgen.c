#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "cf.h"
#include "common.h"

typedef struct _cf_decimal_gen cf_decimal_gen;
struct _cf_decimal_gen {
    mpz_t a, b, c, d;
    cf * x;
};

int cf_decimal_gen_next(cf_decimal_gen *g)
{
    unsigned int limit = UINT_MAX;
    mpz_t i0, i1, a, b, c, t1, t2, t3, t4;
    long long p;
    int result = INT_MAX;

    mpz_inits(i0, i1, a, b, c, t1, t2, t3, t4, NULL);
    while (--limit)
    {
        if (mpz_sgn(g->c) != 0)
        {
            mpz_fdiv_q(i1, g->a, g->c);
        }
        else
        {
            mpz_set_ll(i1, LLONG_MAX);
        }
        if (mpz_sgn(g->d) != 0)
        {
            mpz_fdiv_q(i0, g->b, g->d);
        }
        else
        {
            mpz_set_ll(i0, LLONG_MAX);
        }

        if (mpz_cmp(i1, i0) == 0)
        {
            mpz_set(a, g->a);
            mpz_set(b, g->b);

            if (mpz_sgn(g->c) == 0 && mpz_sgn(g->d) == 0)
            {
                result = INT_MAX;
                goto EXIT_FUNC;
            }

            if (mpz_sgn(i1) < 0)
            {
                mpz_sub_ui(i1, i1, 1u);
            }

            mpz_mul(t1, i1, g->c);
            mpz_mul(t2, i1, g->d);

            mpz_sub(t3, a, t1);
            mpz_sub(t4, b, t2);

            mpz_mul_ui(g->a, t3, 10u);
            mpz_mul_ui(g->b, t4, 10u);
            result = mpz_get_si(i1);
            goto EXIT_FUNC;
        }

        p = cf_next_term(g->x);
        if (p == LLONG_MAX && cf_is_finished(g->x))
        {
            mpz_set(g->b, g->a);
            mpz_set(g->d, g->c);
        }
        else
        {
            mpz_set_ll(t1, p);
            mpz_set(a, g->a);
            mpz_set(b, g->c);

            mpz_mul(t2, a, t1);
            mpz_add(g->a, t2, g->b);

            mpz_set(g->b, a);

            mpz_mul(t2, b, t1);
            mpz_add(g->c, t2, g->d);
            mpz_set(g->d, b);
        }
    }
EXIT_FUNC:
    mpz_clears(i0, i1, a, b, c, t1, t2, t3, t4, NULL);
    return result;
}

int cf_decimal_gen_is_finished(const cf_decimal_gen * const g)
{
    return mpz_sgn(g->c) == 0 && mpz_sgn(g->d) == 0;
}

void cf_decimal_gen_free(cf_decimal_gen *g)
{
    cf_free(g->x);
    mpz_clears(g->a, g->b, g->c, g->d, NULL);
    free(g);
}

cf_decimal_gen * cf_decimal_gen_create(const cf * const x)
{
    cf_decimal_gen * g =
        (cf_decimal_gen*)malloc(sizeof(cf_decimal_gen));

    if (!g)
        return NULL;

    mpz_init_set_ui(g->a, 1u);
    mpz_init_set_ui(g->b, 0u);
    mpz_init_set_ui(g->c, 0u);
    mpz_init_set_ui(g->d, 1u);
    g->x = cf_copy(x);
    return g;
}
