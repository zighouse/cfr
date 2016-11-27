#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "cf.h"

/*!
 * continued fration
 *
 * \author xiezhigang
 * \date   2016-09-07
 */
static cf_class _rational_class;

typedef struct _rational rational;
struct _rational {
    cf base;
    fraction current;
};

static long long rational_next_term(struct _cf *c)
{
    long long mod, v;
    rational * r = (rational*) c;

    if (r->current.d == 0ll)
    {
        return LLONG_MAX;
    }

    if (r->current.n >= 0ll)
    {
        v = r->current.n / r->current.d;
        mod = r->current.n % r->current.d;
        r->current.n = r->current.d;
        r->current.d = mod;
    }
    else
    {
        v = r->current.n / r->current.d - 1ll;
        mod = r->current.n % r->current.d;
        r->current.n = r->current.d;
        r->current.d = r->current.d + mod;
    }
    return v;
}

static int rational_is_finished(const cf * c)
{
    rational * r = (rational*) c;
    return r->current.d == 0ll;
}

static void rational_free(struct _cf *c)
{
    rational * r = (rational*) c;
    free(r);
}

static cf * rational_copy(const cf * c)
{
    rational * r = (rational*) c;
    return cf_create_from_fraction(r->current);
}

static cf_class _rational_class = {
    rational_next_term,
    rational_is_finished,
    rational_free,
    rational_copy
};

cf * cf_create_from_fraction(fraction f)
{
    rational * r;
    int sign = 0;

    r = (rational*) malloc(sizeof(rational));
    if (!r)
        return NULL;

    /* init object for class or type */
    r->base.object_class = &_rational_class;

    /* init object for data */
    if (f.n < 0)
    {
        sign = 1;
        f.n = -f.n;
    }
    if (f.d < 0)
    {
        sign = !! sign;
        f.d = -f.d;
    }
    r->current = (fraction){sign ? -f.n : f.n, f.d};

    return &r->base;
}

long long cf_get_gcd(long long a, long long b)
{
    long long gcd;
    cf * c = cf_create_from_fraction((fraction){a, b});
    rational * r = (rational*) c;
    while (!cf_is_finished(c))
    {
        cf_next_term(c);
    }
    gcd = r->current.n;
    cf_free(c);
    return gcd;
}

int cf_compare(const cf *_x, const cf *_y)
{
    const int limit = 100; /* limit terms to compare */
    int cmp, i = 0;
    long long a = 0, b = 0;
    cf * x = cf_copy(_x);
    cf * y = cf_copy(_y);
    while (a == b && !cf_is_finished(x) && !cf_is_finished(y) && i < limit)
    {
        a = cf_next_term(x);
        b = cf_next_term(y);
        ++i;
    }

    if (a == b)
    {
        if ((cf_is_finished(x) && cf_is_finished(y))
            || (!cf_is_finished(x) && !cf_is_finished(y)))
        {
            cmp = 0; // equal
        }
        else
        {
            if (cf_is_finished(x))
            {
                cmp = i ^ 1 ? 1 : -1;
            }
            else
            {
                cmp = i & 1 ? 1 : -1;
            }
        }
    }
    else if (a > b)
    {
        cmp = i & 1 ? 1 : -1;
    }
    else // a < b
    {
        cmp = i ^ 1 ? 1 : -1;
    }

    cf_free(x);
    cf_free(y);

    return cmp;
}

fraction rational_best_in(const cf* cf1, const cf* cf2)
{
    long long *n, a1, a2;
    size_t size = 32, count = 0;
    cf * c1 = cf_copy(cf1);
    cf * c2 = cf_copy(cf2);
    int is_done = 0;

#define ENLARGE_BUFFER                                            \
    do { if (count >= size - 1)                                   \
    {                                                             \
        /* enlarge buffer */                                      \
        long long *m = realloc(n, size * 2 * sizeof(long long));  \
        if (!m) break; /* no memory */                            \
        size *= 2;                                                \
        n = m;                                                    \
    } } while (0)

    n = (long long *)malloc(size * sizeof(long long));
    if (n != NULL)
    {
        while (!cf_is_finished(c1) && !cf_is_finished(c2))
        {
            a1 = cf_next_term(c1);
            a2 = cf_next_term(c2);
            if (a1 == a2)
            {
                ENLARGE_BUFFER;
                n[count] = a1;
                count++;
            }
            else
            {
                if (count % 2 == 1)
                {
                    ENLARGE_BUFFER;
                    n[count] = (a1 < a2 ? a1 : a2) + 1;
                    count++;
                }
                else
                {
                    ENLARGE_BUFFER;
                    n[count] = (a1 < a2 ? a1 : a2) + 1;
                    count++;
                    // TODO 0.87 and 0.88
                }
                is_done = 1;
                break;
            }
        }
        if (!is_done)
        {
            /* assumpt: c1 < c2 */
            if (count % 2 == 0)
            {
                if (!cf_is_finished(c1))
                {
                    a1 = cf_next_term(c1);
                    ENLARGE_BUFFER;
                    n[count] = a1 + 1;
                    count++;
                }
                //else
                //{
                //    // TODO
                //    printf("TODO 1: count: %ld: a1: %lld, %s; a2: %lld, %s\n", count,
                //           a1, (cf_is_finished(c1) ? "END" : "..."),
                //           a1, (cf_is_finished(c2) ? "END" : "..."));
                //}
            }
            //else
            //{
            //    // TODO
            //    printf("TODO 2: count: %ld: a1: %lld, %s; a2: %lld, %s\n", count,
            //           a1, (cf_is_finished(c1) ? "END" : "..."),
            //           a1, (cf_is_finished(c2) ? "END" : "..."));
            //}
        }
    }

    cf_free(c1);
    cf_free(c2);

    if (count)
    {
        cf * c = cf_create_from_terms(n, count);
        cf_converg_gen * gen = cf_converg_gen_create(c);
        cf_converg_term conv = {0};
        while (!cf_is_finished(gen))
        {
            conv = cf_next_term(gen);
        }
        cf_free(c);
        cf_free(gen);
        free(n);
        return conv.convergent;
    }

    if (n)
        free(n);
    return (fraction){0ll,1ll};
#undef ENLARGE_BUFFER
}


char * cf_convert_to_string_canonical(const cf *c, int max_terms)
{
    cf * x = cf_copy(c);
    char buf[64], *p, *result;
    int size, count;
    int realloced = 0;

    p = buf;
    size = sizeof(buf);
    count = 0;
    realloced = 0;

    while (max_terms > 0 && !cf_is_finished(x))
    {
        long long coef = cf_next_term(x);
        int chars = snprintf(p + count, size - count,
                             count ? " %lld" : "[%lld", coef);
        if (!cf_is_finished(x))
        {
            snprintf(p + count + chars, size - count - chars,
                     count ? "," : ";");
            ++count;
        }
        count += chars;
        --max_terms;
        // reallocate new buffer to contain the too long string.
        if (count > size - 7)
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
        if (!cf_is_finished(x))
        {
            strcpy(p+count, " ...]");
        }
        else
        {
            strcpy(p+count, "]");
        }
    }
    else
    {
        strcpy(p, "[NAN]");
    }

    result = strdup(p);
    if (realloced)
    {
        free(p);
    }
    cf_free(x);

    return result;
}
