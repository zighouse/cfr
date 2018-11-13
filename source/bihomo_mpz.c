/**
 * high precision bihomographic function implementation.
 *
 * \author xiezhigang
 * \date 2018-11-13
 */
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

#include "cf.h"
#include "common.h"
#include "integer.h"

static cf_class _bihomo_mpz_class;

typedef struct _bihomo_mpz bihomo_mpz;
struct _bihomo_mpz {
    cf base;
    integer_t a, b, c, d, e, f, g, h;
    cf *x, *y;
};

static long long bihomo_mpz_next_term(cf *_c)
{
    integer_t a, b, c, d, e, f, g, h;
    integer_t ixy, ix, iy, i0;
    integer_t dxyy, dxyx, d0y, d0x;
    integer_t A, B, C, D, E, F, G, H;
    integer_t t1, t2, t3, t4;
    integer_t term;
    integer_ptr p1, p2;
    unsigned limit = 10000;
    long long result = LLONG_MAX;

    bihomo_mpz * bh = (bihomo_mpz*) _c;

    integer_inits_pre(bh->a->precision, a, b, c, d, e, f, g, h, NULL);
    integer_inits_pre(bh->a->precision, ixy, ix, iy, i0, NULL);
    integer_inits_pre(bh->a->precision, dxyy, dxyx, d0y, d0x, NULL);
    integer_inits_pre(bh->a->precision, A, B, C, D, E, F, G, H, NULL);
    integer_inits_pre(bh->a->precision, t1, t2, t3, t4, term, NULL);

    while (--limit)
    {
        int input_x;

        if (integer_is_zero(bh->e) &&
            integer_is_zero(bh->f) &&
            integer_is_zero(bh->g) &&
            integer_is_zero(bh->h)) 
        {
            result = LLONG_MAX;
            goto exit_func;
        }

        do {

            if (integer_is_zero(bh->f))
            {
                input_x = integer_is_zero(bh->e);
                break;
            }
            integer_div(ix, bh->b, bh->f);

            if (integer_is_zero(bh->g))
            {
                input_x = !integer_is_zero(bh->e);
                break;
            }
            integer_div(iy, bh->c, bh->g);

            integer_div(ixy, bh->a, bh->e);
            integer_div(i0, bh->d, bh->h);

            if (integer_equals(ixy, ix) &&
                integer_equals(ix, iy) &&
                integer_equals(iy, i0))
            {
                bool is_overflow = false;
                /* output a term */
                if (integer_sgn(ixy) < 0)
                {
                    integer_dec(ixy, 1);
                }

                integer_set(a, bh->a);
                integer_set(b, bh->b);
                integer_set(c, bh->c);
                integer_set(d, bh->d);
                integer_set(bh->a, bh->e);
                integer_set(bh->b, bh->f);
                integer_set(bh->c, bh->g);
                integer_set(bh->d, bh->h);

                integer_mul(e, ixy, bh->e); is_overflow |= integer_is_overflow(e);
                integer_mul(f, ixy, bh->f); is_overflow |= integer_is_overflow(f);
                integer_mul(g, ixy, bh->g); is_overflow |= integer_is_overflow(g);
                integer_mul(h, ixy, bh->h); is_overflow |= integer_is_overflow(h);

                if (is_overflow)
                {
                    integer_set_int32(bh->e, 0);
                    integer_set_int32(bh->f, 0);
                    integer_set_int32(bh->g, 0);
                    integer_set_int32(bh->h, 0);
                }
                else
                {
                    integer_sub(bh->e, a, e);
                    integer_sub(bh->f, b, f);
                    integer_sub(bh->g, c, g);
                    integer_sub(bh->h, d, h);
                }

                result = integer_get_int64(ixy);
                goto exit_func;
            }

            if (integer_equals(ixy, ix) &&
                !integer_is_zero(ix))
            {
                input_x = 1;
                break;
            }

            if (integer_equals(ixy, iy) &&
                !integer_is_zero(iy))
            {
                input_x = 0;
                break;
            }

            integer_diff(dxyy, ixy, iy);
            integer_diff(dxyx, ixy, ix);

            if (integer_is_zero(bh->h))
            {
                input_x = integer_cmp(dxyy, dxyx) > 0;
                break;
            }

            integer_diff(d0y, i0, iy);
            integer_diff(d0x, i0, ix);

            if (integer_is_zero(bh->e))
            {
                input_x = integer_cmp(d0y, d0x) > 0;
                break;
            }

            p1 = integer_max(dxyy, d0x);
            p2 = integer_max(dxyx, d0y);
            input_x = integer_cmp(p1, p2) > 0;
        } while (0);

        if (input_x)
        {
            long long p;

            p = cf_next_term(bh->x);
            if (p == LLONG_MAX && cf_is_finished(bh->x))
            {
                integer_set(bh->c, bh->a);
                integer_set(bh->d, bh->b);
                integer_set(bh->g, bh->e);
                integer_set(bh->h, bh->f);
            }
            else
            {
                bool is_overflow = false;
                integer_set(a, bh->a);
                integer_set(b, bh->b);
                integer_set(e, bh->e);
                integer_set(f, bh->f);
                integer_set_int64(term, p);

                is_overflow = false;

                integer_mul(t1, term, a); is_overflow |= integer_is_overflow(t1);
                integer_mul(t2, term, b); is_overflow |= integer_is_overflow(t2);
                integer_mul(t3, term, e); is_overflow |= integer_is_overflow(t3);
                integer_mul(t4, term, f); is_overflow |= integer_is_overflow(t4);
                
                integer_add(A, t1, bh->c); is_overflow |= integer_is_overflow(A);
                integer_add(B, t2, bh->d); is_overflow |= integer_is_overflow(B);
                integer_set(C, a);
                integer_set(D, b);

                integer_add(E, t3, bh->g); is_overflow |= integer_is_overflow(E);
                integer_add(F, t4, bh->h); is_overflow |= integer_is_overflow(F);
                integer_set(G, e);
                integer_set(H, f);

                if (is_overflow) /* {{{ handle overflow exception */
                {
                    integer_t ret;
                    integer_t divae, divbf, divcg;

                    integer_init2(ret, bh->a->precision);
                    integer_inits_pre(bh->a->precision, divae, divbf, divcg, NULL);
                    if (integer_is_zero(bh->e) || integer_is_zero(bh->f) || integer_is_zero(bh->g))
                    {
                        result = LLONG_MAX;
                        integer_set_int64(ret, LLONG_MAX);
                        ret->infinite = true;
                        ret->overflow = true;
                    }
                    else
                    {
                        integer_ptr maxn;
                        integer_div(divae, bh->a, bh->e);
                        integer_div(divbf, bh->b, bh->f);
                        integer_div(divcg, bh->c, bh->g);
                        maxn = integer_max(divae, integer_max(divbf, divcg));
                        integer_set(ret, maxn);
                        result = integer_get_int64(maxn);
                    }

                    // pre-output
                    {
                        integer_set(a, bh->a);
                        integer_set(b, bh->b);
                        integer_set(c, bh->c);
                        integer_set(d, bh->d);
                        integer_set(bh->a, bh->e);
                        integer_set(bh->b, bh->f);
                        integer_set(bh->c, bh->g);
                        integer_set(bh->d, bh->h);

                        is_overflow = false;

                        integer_mul(t1, ret, bh->e); is_overflow |= integer_is_overflow(t1);
                        integer_mul(t2, ret, bh->f); is_overflow |= integer_is_overflow(t2);
                        integer_mul(t3, ret, bh->g); is_overflow |= integer_is_overflow(t3);
                        integer_mul(t4, ret, bh->h); is_overflow |= integer_is_overflow(t4);

                        integer_sub(e, a, t1);
                        integer_sub(f, b, t2);
                        integer_sub(g, c, t3);
                        integer_sub(h, d, t4);

                        if (is_overflow)
                        {
                            integer_set_int32(bh->e, 0);
                            integer_set_int32(bh->f, 0);
                            integer_set_int32(bh->g, 0);
                            integer_set_int32(bh->h, 0);
                        }
                        else
                        {
                            is_overflow = false;

                            integer_mul(t1, term, a); is_overflow |= integer_is_overflow(t1);
                            integer_mul(t2, term, b); is_overflow |= integer_is_overflow(t2);
                            integer_mul(t3, term, e); is_overflow |= integer_is_overflow(t3);
                            integer_mul(t4, term, f); is_overflow |= integer_is_overflow(t4);

                            integer_add(A, t1, c); is_overflow |= integer_is_overflow(A);
                            integer_add(B, t2, d); is_overflow |= integer_is_overflow(B);
                            integer_set(C, a);
                            integer_set(D, b);

                            integer_add(E, t3, g); is_overflow |= integer_is_overflow(E);
                            integer_add(F, t4, h); is_overflow |= integer_is_overflow(F);
                            integer_set(G, e);
                            integer_set(H, f);

                            if (is_overflow)
                            {
                                integer_set_int32(bh->e, 0);
                                integer_set_int32(bh->f, 0);
                                integer_set_int32(bh->g, 0);
                                integer_set_int32(bh->h, 0);
                            }
                            else
                            {
                                integer_set(bh->a, A);
                                integer_set(bh->b, B);
                                integer_set(bh->c, C);
                                integer_set(bh->d, D);
                                integer_set(bh->e, E);
                                integer_set(bh->f, F);
                                integer_set(bh->g, G);
                                integer_set(bh->h, H);
                            }
                        }
                    }
                    integer_clear(ret);
                    integer_clears(divae, divbf, divcg, NULL);
                    goto exit_func;
                } /* overflow exception handled }}} */
                else
                {
                    integer_set(bh->a, A);
                    integer_set(bh->b, B);
                    integer_set(bh->c, C);
                    integer_set(bh->d, D);
                    integer_set(bh->e, E);
                    integer_set(bh->f, F);
                    integer_set(bh->g, G);
                    integer_set(bh->h, H);
                }
            }
        }
        else
        {
            long long p;

            p = cf_next_term(bh->y);
            if (p == LLONG_MAX && cf_is_finished(bh->y))
            {
                integer_set(bh->b, bh->a);
                integer_set(bh->d, bh->c);
                integer_set(bh->f, bh->e);
                integer_set(bh->h, bh->g);
            }
            else
            {
                bool is_overflow = false;

                integer_set(a, bh->a);
                integer_set(c, bh->c);
                integer_set(e, bh->e);
                integer_set(g, bh->g);
                integer_set_int64(term, p);

                is_overflow = false;
                integer_mul(t1, term, a); is_overflow |= integer_is_overflow(t1);
                integer_mul(t2, term, c); is_overflow |= integer_is_overflow(t2);
                integer_mul(t3, term, e); is_overflow |= integer_is_overflow(t3);
                integer_mul(t4, term, g); is_overflow |= integer_is_overflow(t4);

                integer_add(A, t1, bh->b); is_overflow |= integer_is_overflow(A);
                integer_set(B, a);
                integer_add(C, t2, bh->d); is_overflow |= integer_is_overflow(C);
                integer_set(D, c);

                integer_add(E, t3, bh->f); is_overflow |= integer_is_overflow(E);
                integer_set(F, e);
                integer_add(G, t4, bh->h); is_overflow |= integer_is_overflow(G);
                integer_set(H, g);

                if (is_overflow)  /* {{{ handle overflow exception */
                {
                    integer_t ret;
                    integer_t divae, divbf, divcg;

                    integer_init2(ret, bh->a->precision);
                    integer_inits_pre(bh->a->precision, divae, divbf, divcg, NULL);
                    if (integer_is_zero(bh->e) || integer_is_zero(bh->f) || integer_is_zero(bh->g))
                    {
                        result = LLONG_MAX;
                        integer_set_int64(ret, LLONG_MAX);
                        ret->infinite = true;
                        ret->overflow = true;
                    }
                    else
                    {
                        integer_ptr maxn;
                        integer_div(divae, bh->a, bh->e);
                        integer_div(divbf, bh->b, bh->f);
                        integer_div(divcg, bh->c, bh->g);
                        maxn = integer_max(divae, integer_max(divbf, divcg));
                        integer_set(ret, maxn);
                        result = integer_get_int64(maxn);
                    }

                    // pre-output
                    {
                        integer_set(a, bh->a);
                        integer_set(b, bh->b);
                        integer_set(c, bh->c);
                        integer_set(d, bh->d);
                        integer_set(bh->a, bh->e);
                        integer_set(bh->b, bh->f);
                        integer_set(bh->c, bh->g);
                        integer_set(bh->d, bh->h);

                        is_overflow = false;

                        integer_mul(t1, ret, bh->e); is_overflow |= integer_is_overflow(t1);
                        integer_mul(t2, ret, bh->f); is_overflow |= integer_is_overflow(t2);
                        integer_mul(t3, ret, bh->g); is_overflow |= integer_is_overflow(t3);
                        integer_mul(t4, ret, bh->h); is_overflow |= integer_is_overflow(t4);

                        integer_sub(e, a, t1);
                        integer_sub(f, b, t2);
                        integer_sub(g, c, t3);
                        integer_sub(h, d, t4);

                        if (is_overflow)
                        {
                            integer_set_int32(bh->e, 0);
                            integer_set_int32(bh->f, 0);
                            integer_set_int32(bh->g, 0);
                            integer_set_int32(bh->h, 0);
                        }
                        else
                        {
                            is_overflow = false;

                            integer_mul(t1, term, a); is_overflow |= integer_is_overflow(t1);
                            integer_mul(t2, term, c); is_overflow |= integer_is_overflow(t2);
                            integer_mul(t3, term, e); is_overflow |= integer_is_overflow(t3);
                            integer_mul(t4, term, g); is_overflow |= integer_is_overflow(t4);

                            integer_add(A, t1, b); is_overflow |= integer_is_overflow(A);
                            integer_set(B, a);
                            integer_add(C, t2, d); is_overflow |= integer_is_overflow(C);
                            integer_set(D, c);

                            integer_add(E, t3, f); is_overflow |= integer_is_overflow(E);
                            integer_set(F, e);
                            integer_add(G, t4, h); is_overflow |= integer_is_overflow(G);
                            integer_set(H, g);

                            if (is_overflow)
                            {
                                integer_set_int32(bh->e, 0);
                                integer_set_int32(bh->f, 0);
                                integer_set_int32(bh->g, 0);
                                integer_set_int32(bh->h, 0);
                            }
                            else
                            {
                                integer_set(bh->a, A);
                                integer_set(bh->b, B);
                                integer_set(bh->c, C);
                                integer_set(bh->d, D);
                                integer_set(bh->e, E);
                                integer_set(bh->f, F);
                                integer_set(bh->g, G);
                                integer_set(bh->h, H);
                            }
                        }
                    }
                    integer_clear(ret);
                    integer_clears(divae, divbf, divcg, NULL);
                    goto exit_func;
                } /* overflow exception is handled }}} */
                else
                {
                    integer_set(bh->a, A);
                    integer_set(bh->b, B);
                    integer_set(bh->c, C);
                    integer_set(bh->d, D);
                    integer_set(bh->e, E);
                    integer_set(bh->f, F);
                    integer_set(bh->g, G);
                    integer_set(bh->h, H);
                }
            }
        }
    }
exit_func:
    integer_clears(a, b, c, d, e, f, g, h, NULL);
    integer_clears(ixy, ix, iy, i0, NULL);
    integer_clears(dxyy, dxyx, d0y, d0x, NULL);
    integer_clears(A, B, C, D, E, F, G, H, NULL);
    integer_clears(t1, t2, t3, t4, term, NULL);

    return result;
}

static int bihomo_mpz_is_finished(const cf * c)
{
    bihomo_mpz * h = (bihomo_mpz*) c;
    return integer_is_zero(h->e) && integer_is_zero(h->f) &&
           integer_is_zero(h->g) && integer_is_zero(h->h);
}

static void bihomo_mpz_free(cf *c)
{
    bihomo_mpz * h = (bihomo_mpz*) c;
    integer_clears(h->a, h->b, h->c, h->d, h->e, h->f, h->g, h->h, NULL);
    cf_free(h->x);
    cf_free(h->y);
    free(h);
}

static cf * bihomo_mpz_copy(const cf * c)
{
    bihomo_mpz * h = (bihomo_mpz*) c;
    bihomo_mpz * bh = (bihomo_mpz*)malloc(sizeof(bihomo_mpz));

    if (!bh)
        return NULL;

    bh->base.object_class = &_bihomo_mpz_class;

    integer_inits_pre(h->a->precision,
                      bh->a, bh->b, bh->c, bh->d, bh->e, bh->f, bh->g, bh->h,
                      NULL);
    integer_set(bh->a, h->a);
    integer_set(bh->b, h->b);
    integer_set(bh->c, h->c);
    integer_set(bh->d, h->d);
    integer_set(bh->e, h->e);
    integer_set(bh->f, h->f);
    integer_set(bh->g, h->g);
    integer_set(bh->h, h->h);
    bh->x = cf_copy(h->x);
    bh->y = cf_copy(h->y);
    return &bh->base;
}

static cf_class _bihomo_mpz_class = {
    bihomo_mpz_next_term,
    bihomo_mpz_is_finished,
    bihomo_mpz_free,
    bihomo_mpz_copy
};

cf * cf_create_from_bihomo_pre(const cf * x, const cf * y,
                               long long a, long long b, long long c, long long d,
                               long long e, long long f, long long g, long long h,
                               unsigned precision)
{
    bihomo_mpz * bh = (bihomo_mpz*)malloc(sizeof(bihomo_mpz));

    if (!bh)
        return NULL;

    bh->base.object_class = &_bihomo_mpz_class;

    integer_init2_with_int64(bh->a, a, precision);
    integer_init2_with_int64(bh->b, b, precision);
    integer_init2_with_int64(bh->c, c, precision);
    integer_init2_with_int64(bh->d, d, precision);
    integer_init2_with_int64(bh->e, e, precision);
    integer_init2_with_int64(bh->f, f, precision);
    integer_init2_with_int64(bh->g, g, precision);
    integer_init2_with_int64(bh->h, h, precision);
    bh->x = cf_copy(x);
    bh->y = cf_copy(y);
    return &bh->base;
}

/* vim:set fdm=marker: */
