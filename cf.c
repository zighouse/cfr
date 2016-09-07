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

struct _rational {
    cf base;
    long long * term;
    long nterm;
    long i;
};
typedef struct _rational rational;

static long long rational_next_term(cf *c)
{
    rational *r = (rational *)c;
    if (r->i >= r->nterm)
        return LLONG_MAX;
    return r->term[r->i++];
}

static int rational_is_finished(cf *c)
{
    rational *r = (rational *)c;
    return r->i >= r->nterm;
}

static void rational_reset(cf *c)
{
    rational *r = (rational*)c;
    r->i = 0;
}

static void rational_free(cf *c)
{
    rational * r = (rational *)c;
    free(r->term);
    free(r);
}

cf * cf_create_from_fraction(fraction f)
{
    rational *r;
    long long buff[512];
    int index, sign;
    long long mod;

    r = (rational*) malloc(sizeof(rational));
    if (!r)
        return NULL; /* no memory */

    r->term = NULL;
    r->nterm = 0;
    r->i = 0;
    r->base.next_term = rational_next_term;
    r->base.is_finished = rational_is_finished;
    r->base.reset = rational_reset;
    r->base.free = rational_free;

    sign = 0;
    if (f.n < 0)
    {
        f.n = -f.n;
        sign = 1;
    }
    if (f.d < 0)
    {
        f.d = -f.d;
        sign = !sign;
    }

    index = 0;

    // canonicalize for menus
    if (sign)
    {
        mod = f.n % f.d;
        if (mod == 0)
        {
            buff[index++] = -f.n / f.d;
            /* complete */
            f.d = mod;
        }
        else
        {
            long long t = f.d;
            buff[index++] = -f.n / f.d - 1;
            f.n = t;
            f.d = f.d - mod;
        }
    }

    while (f.d)
    {
        buff[index++] = f.n / f.d;

        mod = f.n % f.d;
        f.n = f.d;
        f.d = mod;

        if (index == 256)
        {
            // rewind buffer
            long long * term = malloc((r->nterm + index) * sizeof(long long));
            if (!term)
                return NULL; /* no memory */

            if (r->nterm > 0 && r->term)
            {
                memcpy(term, r->term, r->nterm * sizeof(long long));
                free(r->term);
            }
            memcpy(&term[r->nterm], buff, index * sizeof(long long));
            r->nterm += index;
            r->term = term;
            index = 0;
        }
    }

    {
        // rewind buffer
        long long * term = malloc((r->nterm + index) * sizeof(long long));
        if (!term)
            return NULL; /* no memory */

        if (r->nterm > 0 && r->term)
        {
            memcpy(term, r->term, r->nterm * sizeof(long long));
            free(r->term);
        }
        memcpy(&term[r->nterm], buff, index * sizeof(long long));
        r->nterm += index;
        r->term = term;
    }

    return &r->base;
}
