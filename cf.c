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

struct _rational {
    cf base;
    fraction current;
};
typedef struct _rational rational;

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

static int rational_is_finished(struct _cf *c)
{
    rational * r = (rational*) c;
    return r->current.d == 0ll;
}

static void rational_free(struct _cf *c)
{
    rational * r = (rational*) c;
    free(r);
}

static cf_class _rational_class = {
    rational_next_term,
    rational_is_finished,
    rational_free
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

// vim:set fdm=marker:
