#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <float.h>

#include "cf.h"

static
fraction double2fraction(double x, int * exp)
{
    char *d, *p, *end;
    const int size = sizeof(x);
    char text[65] = {0};
    int i, j;
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
    d = (char*) &x;

    // convert data of double to big-endian binary format
    for (i = 0; i < size; ++i)
    {
        char buf[9] = {0};
        for (j = 0; j < 8; ++j)
        {
#if __BYTE_ORDER == __LITTLE_ENDIAN
            buf[7 - j] = (d[size - i - 1] & 1) + '0';
            d[size - i - 1] >>= 1;
#else
            buf[7 - j] = (d[i] & 1) + '0';
            d[i] >>= 1;
#endif
        }
        strcat(text, buf);
    }

    // mantisa for double is from bit[12], and the first bit is always 1 and omited.
    ret = (fraction) {x < 0.0 ? -1ll : 1ll, 2ll};
    end = strrchr(text, '1');
    for (p = &text[12]; p <= end; ++p)
    {
        ret.n <<= 1;
        ret.d <<= 1;
        ret.n |= (p[0] - '0');
    }

    return ret;
}

cf * cf_create_from_float(double d)
{
    cf * ret;
    int exp;

    fraction rat = double2fraction(d, &exp);
    cf * x = cf_create_from_fraction(rat);
    if (exp > 0)
    {
        ret = cf_create_from_homographic(x, 1ll << exp, 0ll, 0ll, 1ll);
    }
    else
    {
        ret = cf_create_from_homographic(x, 1ll, 0ll, 0ll, 1ll<<-exp);
    }
    cf_free(x);
    return ret;
}
