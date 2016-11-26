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

gcf * gcf_create_from_string_float(const char * float_str)
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
    gfs->str = canonical_float_string(float_str);
    return &gfs->base;
}

cf * cf_create_from_string_float(const char * float_str)
{
    gcf * g;
    cf * c;

    g = gcf_create_from_string_float(float_str);
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

char * canonical_float_string(const char * float_str)
{
    char * ret;
    char local_buf[32];
    char * buf = local_buf;
    int size = sizeof(local_buf);
    const char * p;

    int idx = 0;
    int has_sign = 0, has_decimal = 0, has_number = 0;

    if (float_str != NULL)
        for (p = float_str; *p; ++p)
        {
            if (idx >= size -1)
            {
                // enlarge buf
                char * new_buf = (char *)malloc(sizeof(char) * size * 2);
                memcpy(new_buf, buf, size);
                if (buf != local_buf)
                {
                    free(buf);
                }
                buf = new_buf;
                size *= 2;
            }

            if (*p == '-')
            {
                if (has_sign)
                {
                    break;
                }
                has_sign = 1;
                buf[idx++] = *p;
            }
            else if (*p == '+')
            {
                if (has_sign)
                {
                    break;
                }
                has_sign = 1;
            }
            else if (*p == '.')
            {
                if (!has_number || has_decimal)
                {
                    break;
                }
                has_decimal = 1;
                buf[idx++] = *p;
            }
            else if (*p == ' ')
            {
                if (has_number)
                {
                    break;
                }
            }
            else if (*p >= '0' && *p <= '9')
            {
                buf[idx++] = *p;
                has_number = 1;
                has_sign = 1;
            }
            else
            {
                break;
            }
        }

    if (idx)
    {
        buf[idx] = '\0';
    }

    if (!has_number)
    {
        strcpy(buf, "0");
    }

    ret = strdup(buf);
    if (buf != local_buf)
    {
        free(buf);
    }
    return ret;
}

/*
 * add a digit to a canonical float string at a given location.
 *
 * @result    to store the result if size is allowed, or return error.
 * @size      is the buffer size of result (including ending nil).
 * @float_str is the canonical float string to add a digit.
 * @digit     is the digit (-9~9) to be added into float.
 * @location  is position to decimal point where the digit should be
 *            added into the float string, positive or zero is left to
 *            dot, and negative is right to dot.
 *
 * return 0 if succeed, otherwise error ocurs.
 */
int float_string_add_digit(char *result, int size,
                           const char *float_str,
                           int digit, int location)
{
    const char *f1, *dot;
    char *f2, value;
    int len, pos, i;

    if (!result)
        return 1;

    if (!float_str)
        return 1;

    if (digit > 9 || digit < -9)
        return 1;

    if (float_str[0] == '-')
    {
        /* negative */
        int err;
        err = float_string_add_digit(result+1, size-1,
                                     float_str+1, -digit, location);
        if (err)
            return err;
        result[0] = '-';
        if (result[1] == '-')
        {
            for (i = 0; i < size - 1 && result[i+2] != '\0'; ++i)
            {
                result[i] = result[i+2];
            }
            result[i] = '\0';
        }
        return err;
    }

    f1 = float_str;
    len = strlen(f1);
    dot = strchr(f1, '.');
    if (dot == NULL)
        dot = f1;
    pos = dot - f1 - location - (location > 0 ? 1 : 0);
    if (pos < 0 || pos > len - 1)
        return 1;

    f2 = result + 1;
    if (size < len + 1)
        return 1;

    for (i = len; i > pos; --i)
    {
        f2[i] = f1[i];
    }

    for (i = pos; i >= 0; --i)
    {
        if (f1[i] == '.')
        {
            f2[i] = f1[i];
            continue;
        }
        else
        {
            value = f1[i] + digit;
            if (value > '9')
            {
                digit = 1;
                value -= 10;
            }
            else if (value < '0')
            {
                digit = -1;
                value += 10;
            }
            else
            {
                digit = 0;
            }
            f2[i] = value;
        }
    }

    if (digit > 0)
    {
        result[0] = digit + '0';
    }
    else if (digit == 0)
    {
        for (i = 0; f2[i] != '\0'; ++i)
        {
            result[i] = f2[i];
        }
        result[i] = f2[i];
    }
    else //if (digit < 0)
    {
        /* substract the borrowed 10 down to end */
        result[0] = '-';
        for (i = 0; f2[i] != '\0'; ++i)
        {
            if (f2[i] != '.')
                f2[i] = '0' + '9' - f2[i];
        }
        f2[i-1]++;
    }
    return 0;
}

fraction rational_best_for(const char * f)
{
    char * f0, * f1, * f2, *dot_pos;
    int len, i;
    cf * c1, * c2;
    fraction r;

    f0 = canonical_float_string(f);
    len = strlen(f0);
    dot_pos = strchr(f0, '.');
    f0 = realloc(f0, len + 2 + (dot_pos ? 0 : 1));
    if (dot_pos == NULL)
    {
        f0[len++] = '.';
    }
    f0[len] = '0';
    f0[len+1] = '\0';
    f1 = (char *) malloc(len + 5);
    f2 = (char *) malloc(len + 5);
    dot_pos = strchr(f0, '.');
    if (dot_pos)
    {
        i = 1 - strlen(dot_pos);
    }
    else
    {
        i = -1;
    }
    float_string_add_digit(f1, len + 4, f0, -5, i);
    float_string_add_digit(f2, len + 4, f0,  5, i);
    c1 = cf_create_from_string_float(f1);
    c2 = cf_create_from_string_float(f2);
    r = rational_best_in(c1, c2);

    /* clean up */
    cf_free(c1);
    cf_free(c2);
    free(f1);
    free(f2);
    free(f0);
    return r;
}
