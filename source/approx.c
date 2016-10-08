#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "cf.h"

typedef struct _cf_approx_priv cf_approx_priv;
struct _cf_approx_priv {
    cf_approx base;
    cf *c;
    long long m[4];
    long long ai[2];
    fraction s[2];
    int idx, finished;
};

static
cf_approx_term cf_approx_next_term(cf_approx * approx)
{
    cf_approx_priv *s = (cf_approx_priv*)approx;
    cf_approx_term result = {LLONG_MAX};

    if (s->finished)
        return result;

    if (!cf_is_finished(s->c))
    {
        s->idx = !s->idx;
        s->ai[s->idx] = cf_next_term(s->c);
        s->s[s->idx].n = s->m[0] * s->ai[s->idx] + s->m[2];
        s->s[s->idx].d = s->m[1] * s->ai[s->idx] + s->m[3];

        s->m[2] = s->m[0];
        s->m[3] = s->m[1];

        s->m[0] = s->s[s->idx].n;
        s->m[1] = s->s[s->idx].d;

        result.ai = s->ai[!s->idx];
        result.rational = s->s[!s->idx];
        result.lower_error = s->s[!s->idx].d*(s->s[0].d+s->s[1].d);
        result.upper_error = s->s[0].d*s->s[1].d;
        return result;
    }

    s->finished = 1;

    result.ai = s->ai[s->idx];
    result.rational = s->s[s->idx];
    result.lower_error = LLONG_MAX;
    result.upper_error = LLONG_MAX;

    return result;
}

static
int cf_approx_is_finished(const cf_approx * approx)
{
    return ((const cf_approx_priv *)approx)->finished;
}

static
void cf_approx_free(cf_approx * s)
{
    cf_free(((cf_approx_priv*)s)->c);
    free(s);
}

static
cf_approx * cf_approx_copy(const cf_approx * approx)
{
    cf_approx_priv * s = (cf_approx_priv*) malloc(sizeof(cf_approx_priv));
    if (!s)
        return NULL;

    memcpy(s, approx, sizeof(cf_approx_priv));
    s->c = cf_copy(((const cf_approx_priv *)approx)->c);
    return &s->base;
}

static cf_approx_class _cf_approx_priv_class = {
    cf_approx_next_term,
    cf_approx_is_finished,
    cf_approx_free,
    cf_approx_copy
};

cf_approx * cf_approx_create(const cf * c)
{
    cf_approx_priv * s = (cf_approx_priv*) malloc(sizeof(cf_approx_priv));
    if (!s)
        return NULL;
    s->m[0] = s->m[3] = 1ll;
    s->m[2] = s->m[1] = 0ll;
    s->c = cf_copy(c);
    s->idx = 0;
    s->ai[s->idx] = cf_next_term(s->c);
    s->s[s->idx].n = s->m[0] * s->ai[s->idx] + s->m[2];
    s->s[s->idx].d = s->m[1] * s->ai[s->idx] + s->m[3];
    s->m[2] = s->m[0];
    s->m[3] = s->m[1];
    s->m[0] = s->s[s->idx].n;
    s->m[1] = s->s[s->idx].d;
    s->finished = cf_is_finished(s->c);
    s->base.object_class = &_cf_approx_priv_class;
    return &s->base;
}
