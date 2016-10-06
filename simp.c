#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "cf.h"

struct _cf_simplifier {
    cf *c;
    long long m[4];
    long long ai[2];
    fraction s[2];
    int idx, finished;
};

cf_simplifier * cf_simplifier_create(const cf * const c)
{
    cf_simplifier * s = (cf_simplifier*) malloc(sizeof(cf_simplifier));
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
    return s;
}

int cf_simplifier_next(cf_simplifier * s, long long *ai, fraction *f, error_range * e)
{
    if (s->finished)
        return 0;

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

        if (ai)
            *ai = s->ai[!s->idx];

        if (f)
            *f = s->s[!s->idx];

        if (e)
            *e = (error_range){0,
            (fraction){1, s->s[!s->idx].d*(s->s[0].d+s->s[1].d)},
            (fraction){1, s->s[0].d*s->s[1].d}};
        return 1;
    }

    s->finished = 1;

    if (ai)
        *ai = s->ai[s->idx];

    if (f)
        *f = s->s[s->idx];

    if (e)
        *e = (error_range){3, (fraction){0, 1}, (fraction){0, 1}};

    return 1;
}

int cf_simplifier_is_finished(const cf_simplifier * const s)
{
    return s->finished;
}

void cf_simplifier_free(cf_simplifier * s)
{
    cf_free(s->c);
    free(s);
}
