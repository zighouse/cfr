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
