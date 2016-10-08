#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <float.h>

#include "cf.h"

static
fraction float_to_fraction(double x, int * exp)
{
    char * data;
    fraction ret;

    switch (fpclassify(x))
    {
    case FP_NAN:
        return (fraction){1ll, 0ll};

    case FP_INFINITE:
        return (fraction){x > 0.0 ? 1ll : -1ll, 0ll};

    case FP_ZERO:
    case FP_SUBNORMAL:
        return (fraction){0ll, 1ll};

    case FP_NORMAL:
    default:
        break;
    }

    x = frexp(x, exp);
    data = (char*) &x;

    /*
     * the mantisa for double is from bit[12], and the first bit is always 1
     * and is omited.
     */
    ret.n = (0x000fffffffffffffll & *(long long*)data) | 0x0010000000000000ll;
    ret.d = 0x0020000000000000ll;

    return ret;
}

cf * cf_create_from_float(double d)
{
    cf * ret;
    int exp;

    fraction rat = float_to_fraction(d, &exp);
    cf * x = cf_create_from_fraction(rat);
    if (exp > 0)
    {
        ret = cf_create_from_homographic(x, 1ll << exp, 0ll, 0ll, 1ll);
    }
    else
    {
        ret = cf_create_from_homographic(x, 1ll, 0ll, 0ll, 1ll << -exp);
    }
    cf_free(x);
    return ret;
}

typedef struct _gcf_float_str {
    gcf base;
    char * str;
    char chr;
    int idx;
} gcf_float_str;

static number_pair _gcf_float_str_next_term(gcf *g)
{
    gcf_float_str * gfs = (gcf_float_str*)g;

    if (!gfs->str)
    {
        return (number_pair){1, LLONG_MAX};
    }

    if (!gfs->idx)
    {
        long long n = atoll(gfs->str);
        char *p = strchr(gfs->str, '.'); 
        if (p)
        {
            gfs->idx = p - gfs->str + 1;
        }
        else
        {
            gfs->idx = strlen(gfs->str);
        }
        return (number_pair){1, n};
    }

    if (gfs->chr == 0)
    {
        char chr = gfs->str[gfs->idx];
        if (chr >= '0' && chr <= '9')
        {
            gfs->chr = chr;
            return (number_pair){1ll, 0ll};
        }
        else
        {
            // finished
            return (number_pair){1ll, LLONG_MAX};
        }
    }
    else
    {
        number_pair np = {10ll, gfs->chr - '0'};
        gfs->chr = '\0';
        ++gfs->idx;
        return np;
    }
}

static int _gcf_float_str_is_finished(const gcf *g)
{
    gcf_float_str * gfs = (gcf_float_str*)g;

    if (!gfs->str)
    {
        return 1;
    }

    if (gfs->chr)
    {
        return 0;
    }

    if (gfs->str[gfs->idx] >= '0' && gfs->str[gfs->idx] <= '9' )
    {
        return 0;
    }
    return 1;
}

static void _gcf_float_str_free(gcf *g)
{
    gcf_float_str * gfs = (gcf_float_str*)g;
    free(gfs->str);
    free(gfs);
}

static gcf * _gcf_float_str_copy(const gcf * g)
{
    gcf_float_str * gfs = (gcf_float_str*)g;
    gcf_float_str * gfs_new = (gcf_float_str*)malloc(sizeof(gcf_float_str));
    memcpy(gfs_new, gfs, sizeof(gcf_float_str));
    gfs_new->str = strdup(gfs->str);
    return &gfs_new->base;
}

static gcf_class _gcf_float_str_class = {
    _gcf_float_str_next_term,
    _gcf_float_str_is_finished,
    _gcf_float_str_free,
    _gcf_float_str_copy
};

gcf * gcf_create_from_float_str(const char * float_str)
{
    gcf_float_str * gfs;
    gfs = (gcf_float_str*)malloc(sizeof(gcf_float_str));
    if (!gfs)
    {
        return NULL;
    }

    gfs->base.object_class = &_gcf_float_str_class;
    gfs->chr = '\0';
    gfs->idx = 0;
    gfs->str = strdup(float_str);
    return &gfs->base;
}

cf * cf_create_from_float_str(const char * float_str)
{
    gcf * g;
    cf * c;

    g = gcf_create_from_float_str(float_str);
    if (!g)
    {
        return NULL;
    }

    c = cf_create_from_ghomo(g, 1, 0, 0, 1);
    if (!c)
    {
        return NULL;
    }

    cf_free(g);
    return c;
}
