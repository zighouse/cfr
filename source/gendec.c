#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "cf.h"
#include "common.h"

typedef struct _cf_digit_gen_dec cf_digit_gen_dec;
struct _cf_digit_gen_dec {
    cf_digit_gen base;
    mpz_t a, b, c, d;
    cf * x;
};

static
int cf_digit_gen_dec_next_term(cf_digit_gen *gen)
{
    unsigned int limit = UINT_MAX;
    mpz_t i0, i1, a, b, c, t1, t2, t3, t4;
    long long p;
    int result = INT_MAX;
    cf_digit_gen_dec * g = (cf_digit_gen_dec*)gen;

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

            // FIXME: try to output one digit `0' after finished.
            if (mpz_sgn(g->c) == 0 && mpz_sgn(g->d) == 0)
            {
                //result = INT_MAX;
                result = 0;
                goto EXIT_FUNC;
            }

            if (mpz_sgn(g->a) == 0 && mpz_sgn(g->b) == 0)
            {
                //result = INT_MAX;
                result = 0;
                goto EXIT_FUNC;
            }

            if (mpz_sgn(i1) < 0 && !cf_is_finished(g->x))
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
            if (mpz_sgn(i1) < 0)
            {
                result = -result;
            }
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

static
int cf_digit_gen_dec_is_finished(const cf_digit_gen * gen)
{
    const cf_digit_gen_dec * g = (const cf_digit_gen_dec *)gen;
    return (mpz_sgn(g->c) == 0 && mpz_sgn(g->d) == 0)
        || (mpz_sgn(g->a) == 0 && mpz_sgn(g->b) == 0);
}

static
void cf_digit_gen_dec_free(cf_digit_gen *gen)
{
    cf_digit_gen_dec * g = (cf_digit_gen_dec*)gen;
    cf_free(g->x);
    mpz_clears(g->a, g->b, g->c, g->d, NULL);
    free(g);
}

static
cf_digit_gen * cf_digit_gen_dec_copy(const cf_digit_gen * gen)
{
    const cf_digit_gen_dec * g = (const cf_digit_gen_dec *)gen;
    return cf_digit_gen_create_dec(g->x);
}

static cf_digit_gen_class _cf_digit_gen_dec_class = {
    cf_digit_gen_dec_next_term,
    cf_digit_gen_dec_is_finished,
    cf_digit_gen_dec_free,
    cf_digit_gen_dec_copy
};

cf_digit_gen * cf_digit_gen_create_dec(const cf * x)
{
    cf_digit_gen_dec * g =
        (cf_digit_gen_dec*)malloc(sizeof(cf_digit_gen_dec));

    if (!g)
        return NULL;

    mpz_init_set_ui(g->a, 1u);
    mpz_init_set_ui(g->b, 0u);
    mpz_init_set_ui(g->c, 0u);
    mpz_init_set_ui(g->d, 1u);
    g->x = cf_copy(x);
    g->base.object_class = &_cf_digit_gen_dec_class;
    return &g->base;
}

// FIXME: get an error string if it is negative.
char * cf_convert_to_string_float(const cf *c, int max_digits)
{
    cf_digit_gen * gen;
    char buf[64], *p, *result;
    int size, count, digit;
    int realloced = 0;

    p = buf;
    size = sizeof(buf);
    count = 0;
    realloced = 0;

    gen = cf_digit_gen_create_dec(c);
    while (max_digits > 0 && !cf_is_finished(gen))
    {
        int chars;
        digit = cf_next_term(gen);
        chars = snprintf(p + count, size - count, "%d", digit);
        if (!count && !cf_is_finished(gen))
        {
            snprintf(p + count + chars, size - count - chars, ".");
            ++count;
        }
        count += chars;
        max_digits -= chars;
        // reallocate new buffer to contain the too long string.
        if (count > size - 5)
        {
            int new_size = size << 1;
            char * new_buf = (char*)malloc(new_size);
            if (!new_buf)
            {
                // failed.
                break;
            }
            memcpy(new_buf, p, count);
            if (realloced)
            {
                free(p);
            }
            p = new_buf;
            size = new_size;
            realloced = 1;
        }
    }

    if (count)
    {
        if (!cf_is_finished(gen))
        {
            strcpy(p+count, "...");
        }
        else
        {
            p[count] = '\0';
        }
    }
    else
    {
        strcpy(p, "NAN");
    }

    result = strdup(p);
    if (realloced)
    {
        free(p);
    }
    cf_free(gen);

    return result;
}
