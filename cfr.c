/*
** calculate continued fraction for a fraction or a real number.
** In long long integers by Xie Zhigang, 27 Aug 2016
** With corrections from Arno Formella, May 2008 (http://www.ics.uci.edu/~eppstein/numth/frap.c)
** David Eppstein / UC Irvine / 8 Aug 1993
**
** usage: cfr num [den] [-m maxden]
**   num    is real number to approx, or numerator, integer or float
**   den    [optional] is the denominator, integer or float
**   maxden [optional] is the maximum denominator allowed
**
** based on the theory of continued fractions
** if x = a1 + 1/(a2 + 1/(a3 + 1/(a4 + ...)))
** then best approximation is found by truncating this series
** (with some adjustments in the last term).
**
** Note the fraction can be recovered as the first column of the matrix
**  ( a1 1 ) ( a2 1 ) ( a3 1 ) ...
**  ( 1  0 ) ( 1  0 ) ( 1  0 )
** Instead of keeping the sequence of continued fraction terms,
** we just keep the last partial product of these matrices.
*/

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * inform that ai is calculated,
 * num - numerator of simple fraction at current iteration.
 * den - denominator of simple fraction at current iteration.
 * ai  - continuous fraction factor.
 * gcd - gcd or 0 if is approximation.
 * data - opaque user data
 */
typedef void(*infocb)(long long num, long long den, long long ai, long long gcd,
                      void * data);

static void cfr(long long num, long long den, long long maxden, infocb print, void * data)
{
    long long m[2][2];
    long long ai;

    m[0][0] = m[1][1] = 1ll;
    m[0][1] = m[1][0] = 0ll;

    while (den)
    {
        long long t, mod;

        ai = num / den;
        if (m[1][0] *  ai + m[1][1] > maxden)
        {
            break;
        }

        t = m[0][0] * ai + m[0][1];
        m[0][1] = m[0][0];
        m[0][0] = t;
        t = m[1][0] * ai + m[1][1];
        m[1][1] = m[1][0];
        m[1][0] = t;

        mod = num % den;
        print(m[0][0], m[1][0], ai, mod ? 0 : den, data);
        num = den;
        den = mod;
    }
}

static void help(char * name)
{
    int main(int argc, char ** argv);
    char * argv[4];

    argv[0] = name;
    fprintf(stderr, "lists a serial of simple fractions approximate original fraction or real,\n"
                    "and continuous fraction factors, errors or gcd.\n\n");
    fprintf(stderr, "usage: %s num [den] [-m maxden]\n", name);
    fprintf(stderr, "  num    - is real number , or numerator of a fraction\n");
    fprintf(stderr, "  den    - [optional] is the denominator, integer or another real\n");
    fprintf(stderr, "  maxden - [optional] is the maximum denominator allowed\n");

    fprintf(stderr, "\nfor example:\n\n");

    fprintf(stderr, "$ %s 3.14159 -m 200\n", name);
    argv[1] = "3.14159";
    argv[2] = "-m";
    argv[3] = "200";
    main(4, argv);
    fprintf(stderr, "  result shows the best fraction with den<=200 approx 3.14159 is 355/113,\n");
    fprintf(stderr, "  and the continuous fraction is: 3 + 1/(7 + 1/(15 + 1/(1 + ...))).\n");
    fprintf(stderr, "\n");

    fprintf(stderr, "$ %s 1920 1080\n", name);
    argv[1] = "1920";
    argv[2] = "1080";
    main(3, argv);
    fprintf(stderr, "  result shows: 1920/1080 = 16/9, and the gcd of 1920 and 1080 is 120.\n");
    fprintf(stderr, "\n");

    fprintf(stderr, "- Xie Zhigang, 27 Aug 2016\n");
    fprintf(stderr, "- Arno Formella, May 2008\n");
    fprintf(stderr, "- David Eppstein / UC Irvine / 8 Aug 1993\n");
}

struct my_data {
    long long num;
    long long den;
    long long maxden;
    double x;
    int sign;
    int is_float;
    int index;
};

static void print_result(long long n, long long d, long long ai, long long gcd, void * data)
{
    struct my_data * p = (struct my_data*) data;
    // columns: simp, ai, error or gcd
    if (gcd)
    {
        if (p->is_float)
        { 
            printf("%s%lld/%lld %s%lld err=0\n",
                   (p->sign ? "-" : ""), n, d,
                   (p->sign && !++p->index? "-" : ""), ai);
        }
        else
        {
            printf("%s%lld/%lld %s%lld gcd=%lld\n",
                   (p->sign ? "-" : ""), n, d,
                   (p->sign && !++p->index? "-" : ""), ai,
                   gcd);
        }
    }
    else
    {
        printf("%s%lld/%lld %s%lld err=%e\n",
               (p->sign ? "-" : ""), n, d,
               (p->sign && !++p->index? "-" : ""), ai,
               p->x - (double)n/(double)d);
    }
}

// {{{ parse long-long int from possible floats
static int parsell(char * arg, long long * value, long long * mantisa)
{
    long long v, m;

    do {
        char * endptr;
        v = strtoll(arg, &endptr, 10);
        m = 1;

        if (v == LLONG_MAX || v == LLONG_MIN)
        {
            fprintf(stderr, "wrong range of number: %s\n", arg);
            return 0;
        }

        // posible float
        if (endptr && *endptr)
        {
            if (endptr[0] == '.')
            {
                char buff[100];
                int len;

                len = snprintf(buff, sizeof(buff), "%lld", v);
                len = snprintf(buff + len, sizeof(buff) - len, "%s", endptr + 1);

                v = strtoll(buff, &endptr, 10);

                if (v == LLONG_MAX || v == LLONG_MIN)
                {
                    fprintf(stderr, "wrong range of number: %s\n", arg);
                    return 0;
                }

                if (endptr && *endptr)
                {
                    fprintf(stderr, "parse number failed: %s\n", arg);
                    return 0;
                }

                for (; len > 0; --len)
                {
                    m *= 10;
                }
                break;
            }

            fprintf(stderr, "parse number failed: %s\n", arg);
            return 0;
        }
    } while (0);

    *value = v;
    *mantisa = m;
    return 1;
}
// }}}

int main(int argc, char ** argv)
{
    struct my_data data = {0};
    int has_maxden;
    data.maxden = LLONG_MAX;

    /* {{{ read command line arguments */
    // parse numerator
    if (argc < 2)
    {
        help(argv[0]);
        exit(1);
    }
    if (!parsell(argv[1], &data.num, &data.den))
    {
        exit(1);
    }
    if (data.num < 0)
    {
        data.num = -data.num;
        data.sign = 1;
    }
    else
    {
        data.sign = 0;
    }
    data.is_float = data.den > 1;

    // parse denominator or max-denominator
    if (argc > 2)
    {
        // is max-denominator
        if (strcmp(argv[2], "-m") == 0)
        {
            if (argc < 4)
            {
                help(argv[0]);
                exit(1);
            }
            data.maxden = atoll(argv[3]);
            if (data.maxden < 0)
            {
                fprintf(stderr, "wrong maxden = %lld\n", data.maxden);
                exit(1);
            }
            has_maxden = 1;
        }
        else
        {
            // is denominator
            long long v, r;
            if (!parsell(argv[2], &v, &r))
            {
                exit(1);
            }
            if (v < 0)
            {
                v = -v;
                data.sign = !data.sign;
            }
            if (data.den > r)
            {
                data.den = v * data.den / r;
            }
            else
            {
                data.num = data.num * r / data.den;
                data.den = v;
            }
        }
    }

    // posible maxden
    if (!has_maxden && argc > 3)
    {
        if (strcmp(argv[3], "-m") == 0)
        {
            if (argc < 5)
            {
                help(argv[0]);
                exit(1);
            }
            data.maxden = atoll(argv[4]);
            if (data.maxden < 0)
            {
                fprintf(stderr, "wrong maxden = %lld\n", data.maxden);
                exit(1);
            }
        }
        else
        {
            help(argv[0]);
            exit(1);
        }
    }

    /* }}} */

    data.x = (double)data.num/(double)data.den;

    cfr(data.num, data.den, data.maxden, print_result, &data);

    return 0;
}
// vim:set fdm=marker:
