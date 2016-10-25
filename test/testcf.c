#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "cf.h"

#define FORMAT_FAIL  "[1m[31m"
#define FORMAT_OK    "[1m[32m"
#define FORMAT_END   "[0m"

#define ASSERT(exp)                                                    \
{                                                                      \
    if (exp) {}                                                        \
    else {                                                             \
        printf("[" FORMAT_FAIL "FAILED" FORMAT_END "]\n");             \
        fprintf(stderr, "%s:%d: %s: Assertion `%s' failed.\n",         \
               __FILE__, __LINE__, __func__, #exp);                    \
        return 1;                                                      \
    }                                                                  \
}

#define TEST(case_name)                                                \
{                                                                      \
    printf("TEST %s: ", #case_name);                                   \
    if (test_case_ ##case_name () == 0)                                \
        printf("[" FORMAT_OK "OK" FORMAT_END "]\n");                   \
}

static int test_case_arithmatics(void)
{
    {
        cf * c;

        printf("\n  -16 / 9 = ");

        c = cf_create_from_fraction((fraction){-16, 9});
        while (!cf_is_finished(c))
        {
            printf("%lld ", cf_next_term(c));
        }
        printf("\n");
        cf_free(c);
    }

    {
        cf * c1, *c2;

        printf("\n  x = 16 / 9\n");
        printf("           1x + 0\n");
        printf("       h = ------ = ");

        c1 = cf_create_from_fraction((fraction){16, 9});
        c2 = cf_create_from_homographic(c1, 1, 0, 0, 1);
        while (!cf_is_finished(c2))
        {
            printf("%lld ", cf_next_term(c2));
        }
        printf("\n");
        printf("           0x + 1\n");
        cf_free(c2);
        cf_free(c1);
    }

    {
        cf * c1, *c2;

        printf("\n  x = 16 / 9\n");
        printf("           -x + 0\n");
        printf("       h = ------ = ");

        c1 = cf_create_from_fraction((fraction){16, 9});
        c2 = cf_create_from_homographic(c1, -1, 0, 0, 1);
        while (!cf_is_finished(c2))
        {
            printf("%lld ", cf_next_term(c2));
        }
        printf("\n");
        printf("           0x + 1\n");
        cf_free(c2);
        cf_free(c1);
    }

    {
        cf * c1, *c2;

        printf("\n  x = 16 / 9\n");
        printf("           0x + 1\n");
        printf("       h = ------ = ");

        c1 = cf_create_from_fraction((fraction){16, 9});
        c2 = cf_create_from_homographic(c1, 0, 1, 1, 0);
        while (!cf_is_finished(c2))
        {
            printf("%lld ", cf_next_term(c2));
        }
        printf("\n");
        printf("           1x + 0\n");
        cf_free(c2);
        cf_free(c1);
    }

    {
        cf * c1, *c2;

        printf("\n  x = 16 / 9\n");
        printf("           0x + 1\n");
        printf("       h = ------ = ");

        c1 = cf_create_from_fraction((fraction){16, 9});
        c2 = cf_create_from_homographic(c1, 0, 1, -1, 0);
        while (!cf_is_finished(c2))
        {
            printf("%lld ", cf_next_term(c2));
        }
        printf("\n");
        printf("           -x + 0\n");
        cf_free(c2);
        cf_free(c1);
    }

    {
        cf * c1, *c2;

        printf("\n  x =-16 / 9\n");
        printf("           0x + 1\n");
        printf("       h = ------ = ");

        c1 = cf_create_from_fraction((fraction){-16, 9});
        c2 = cf_create_from_homographic(c1, 0, 1, -1, 0);
        while (!cf_is_finished(c2))
        {
            printf("%lld ", cf_next_term(c2));
        }
        printf("\n");
        printf("           -x + 0\n");
        cf_free(c2);
        cf_free(c1);
    }

    {
        cf * h, *x, *y;
        double fx, fy;
        char *s1, *s2;
        fx = -16.0/9.0;
        fy = 127.0/50.0;

        printf("\n  x =-16 / 9, y = 127 / 50 \n");
        printf("           3xy + 2x +  y + 0\n");
        printf("       h = ----------------- = %f = ",
               (3*fx*fy+2*fx+fy)/(-fx*fy-2*fx-3*fy+4));

        x = cf_create_from_fraction((fraction){-16, 9});
        y = cf_create_from_fraction((fraction){127, 50});
        h = cf_create_from_bihomographic(x, y,
                                         3,  2,  1, 0,
                                         -1, -2, -3, 4);
        s1 = cf_convert_to_string_canonical(h, 20);
        s2 = cf_convert_to_string_float(h, 100);
        while (!cf_is_finished(h))
        {
            printf("%lld ", cf_next_term(h));
        }
        printf("\n");
        printf("           -xy - 2x - 3y + 4\n");
        printf("          CF: %s\n", s1);
        printf("       float: %s\n", s2);

        cf_free(x);
        cf_free(y);
        cf_free(h);
        free(s1);
        free(s2);
    }

    {
        cf * h, *x, *y;
        double fx, fy;
        fx = -16.0/9.0;
        fy = 127.0/50.0;

        printf("\n  x =-16 / 9, y = 127 / 50 \n");
        printf("           3xy + 2x +  y + 4\n");
        printf("       h = ----------------- = %f = ",
               (3*fx*fy+2*fx+fy+4)/(fx*fy+2*fx+3*fy));

        x = cf_create_from_fraction((fraction){-16, 9});
        y = cf_create_from_fraction((fraction){127, 50});
        h = cf_create_from_bihomographic(x, y,
                                         3,  2,  1, 4,
                                         1,  2,  3, 0);
        while (!cf_is_finished(h))
        {
            printf("%lld ", cf_next_term(h));
        }
        printf("\n");
        printf("            xy + 2x + 3y + 0\n");
        cf_free(x);
        cf_free(y);
        cf_free(h);
    }

    return 0;
}

static int test_case_convergent(void)
{
    {
        cf * c;
        cf_converg_gen * gen;

        printf("\n  x = 16 / 9: \n");

        c = cf_create_from_fraction((fraction){16, 9});
        gen = cf_converg_gen_create(c);
        while (!cf_is_finished(gen))
        {
            cf_converg_term t = cf_next_term(gen);

            printf("       %lld, %lld / %lld\n",
                   t.coef, t.convergent.n, t.convergent.d);
        }
        cf_free(c);
        cf_free(gen);
    }

    {
        cf *c;
        cf_converg_gen *s;
        long long numbers[4] = {1, 1, 3, 2};

        printf("\n  x = 1, 1, 3, 2: \n");
        printf("\tcf\tapprox\terror = |x-simple|\n");

        //c = cf_create_from_fraction((fraction){16, 9});
        c = cf_create_from_terms(numbers,
                                 sizeof(numbers)/sizeof(numbers[0]));
        s = cf_converg_gen_create(c);

        while (!cf_is_finished(s))
        {
            cf_converg_term term = cf_next_term(s);
            if (term.lower_error == LLONG_MAX)
            {
                printf("\t%lld\t%lld/%lld\t0\n",
                       term.coef, term.convergent.n, term.convergent.d);
            }
            else
            {
                printf("\t%lld\t%lld/%lld\t(%lld/%lld,%lld/%lld)\n",
                       term.coef, term.convergent.n, term.convergent.d,
                       1ll, term.lower_error,
                       1ll, term.upper_error);
            }
        }

        cf_free(c);
        cf_free(s);
    }
    return 0;
}

static int test_case_calculate_pi_gcf(void)
{
    {
        cf *c;
        gcf *g;
        number_pair pairs[] = {
            {1, 0},
            {4, 1},
            {1, 3},
            {4, 5},
            {9, 7},
            {16, 9},
            {25, 11},
            {36, 13},
            {49, 15},
            {64, 17},
        };

        printf("\n  gcf(pi): ");

        g = gcf_create_from_pairs(pairs, sizeof(pairs)/sizeof(pairs[0]));
        c = cf_create_from_ghomo(g, 1, 0, 0, 1);
        while (!cf_is_finished(g))
        {
            number_pair pn = cf_next_term(g);
            printf("(%lld, %lld) ", pn.a, pn.b);
        }
        printf("\n");
        printf("  cf(pi): ");
        while (!cf_is_finished(c))
        {
            printf(" %lld", cf_next_term(c));
        }
        printf("\n");

        cf_free(g);
        cf_free(c);
    }

    {
        int limit, i;
        gcf *g = gcf_create_from_pi();
        cf * c = cf_create_from_ghomo(g, 1, 0, 0, 1);
        char * s = cf_convert_to_string_canonical(c, 100);

        limit = 10;
        printf("\n  gcf(pi): ");
        for (i = 0; i < limit; ++i)
        {
            number_pair pn = cf_next_term(g);
            printf("(%lld,%lld) ", pn.a, pn.b);
        }
        printf("...\n");

        printf("  cf(pi): %s\n", s);

        cf_free(g);
        cf_free(c);
        free(s);
    }
    return 0;
}

static int test_case_calculate_pi(void)
{
    cf  *c = cf_create_from_pi();
    char * s = cf_convert_to_string_float(c, 100);

    printf("\n  calulate pi to 100 digits as:\n  %s\n", s);

    cf_free(c);
    free(s);
    return 0;
}

static int test_case_convert_from_float(void)
{
    double pi = 3.141592653589793;
    cf  *c;
    char * str;

    c = cf_create_from_float(pi);
    str = cf_convert_to_string_float(c, strlen("3141592653589793"));
    ASSERT( strcmp(str, "3.141592653589793...") == 0 );

    cf_free(c);
    free(str);
    return 0;
}

static int test_case_convert_pi_string(void)
{
    const char * pi = "3.141592653589793238462643383279502884197169399";
    cf  *c;
    char * str1, * str2;

    c = cf_create_from_string_float(pi);
    str1 = cf_convert_to_string_canonical(c, 20);
    str2 = cf_convert_to_string_float(c, 50);

    ASSERT( strstr(str1, "[3; 7, 15, 1, 292, 1, 1, 1, 2, 1, 3, 1, 14") != NULL);
    ASSERT( strcmp(pi, str2) == 0 );

    free(str1);
    free(str2);
    cf_free(c);

    return 0;
}

static int test_case_convert_string_canonical(void)
{
    {
        cf *c = cf_create_from_terms_i(1, 2);
        char * str = cf_convert_to_string_canonical(c, 10);

        ASSERT( strcmp(str, "[2]" ) == 0 );

        free(str);
        cf_free(c);
    }

    {
        cf *c = cf_create_from_terms_i(2, 2, 3);
        char * str = cf_convert_to_string_canonical(c, 10);

        ASSERT( strcmp(str, "[2; 3]" ) == 0 );

        free(str);
        cf_free(c);
    }

    {
        cf *c = cf_create_from_terms_i(3, 2, 3, 4);
        char * str = cf_convert_to_string_canonical(c, 10);

        ASSERT( strcmp(str, "[2; 3, 4]" ) == 0 );

        free(str);
        cf_free(c);
    }

    {
        cf *c = cf_create_from_terms_i(5, 2, 3, 4, 5, 6);
        char * str = cf_convert_to_string_canonical(c, 4);

        ASSERT( strcmp(str, "[2; 3, 4, 5, ...]" ) == 0 );

        free(str);
        cf_free(c);
    }

    {
        cf *c = cf_create_from_terms_i(1, -1);
        char * str = cf_convert_to_string_canonical(c, 10);

        ASSERT( strcmp(str, "[-1]" ) == 0 );

        free(str);
        cf_free(c);
    }

    {
        cf *c = cf_create_from_terms_i(2, -1, 2);
        char * str = cf_convert_to_string_canonical(c, 10);

        ASSERT( strcmp(str, "[-1; 2]" ) == 0 );

        free(str);
        cf_free(c);
    }
    return 0;
}

static int test_case_convert_string_float(void)
{
    {
        cf *c = cf_create_from_terms_i(1, 2);
        char * fl = cf_convert_to_string_float(c, 10);

        ASSERT( strcmp(fl, "2" ) == 0 );

        free(fl);
        cf_free(c);
    }

    {
        cf *c = cf_create_from_terms_i(2, 2, 3);
        char * fl = cf_convert_to_string_float(c, 10);

        ASSERT( strcmp(fl, "2.333333333..." ) == 0 );

        free(fl);
        cf_free(c);
    }

    {
        cf *c = cf_create_from_terms_i(3, 2, 3, 4);
        char * fl = cf_convert_to_string_float(c, 10);

        ASSERT( strcmp(fl, "2.307692307..." ) == 0 );

        free(fl);
        cf_free(c);
    }

    {
        cf *c = cf_create_from_terms_i(5, 2, 3, 4, 5, 6);
        char * fl = cf_convert_to_string_float(c, 10);

        ASSERT( strcmp(fl, "2.308788598..." ) == 0 );

        free(fl);
        cf_free(c);
    }

    {
        cf *c = cf_create_from_terms_i(1, -1);
        char * fl = cf_convert_to_string_float(c, 10);

        ASSERT( strcmp(fl, "-1" ) == 0 );

        free(fl);
        cf_free(c);
    }

    {
        cf *c = cf_create_from_terms_i(2, -1, 2);
        char * fl = cf_convert_to_string_float(c, 10);

        ASSERT( strcmp(fl, "-0.5" ) == 0 );

        free(fl);
        cf_free(c);
    }

    {
        cf *c = cf_create_from_terms_i(3, -1, 2, 3);
        char * fl = cf_convert_to_string_float(c, 10);

        ASSERT( strcmp(fl, "-0.571428571..." ) == 0 );

        free(fl);
        cf_free(c);
    }

    {
        cf *c = cf_create_from_terms_i(4, -1, 2, 3, 4);
        char * fl = cf_convert_to_string_float(c, 10);

        ASSERT( strcmp(fl, "-0.566666666..." ) == 0 );

        free(fl);
        cf_free(c);
    }
    return 0;
}

static int test_case_gcd(void)
{
    long long a, b, gcd;

    {
        a = 1920;
        b = 1080;
        gcd = cf_get_gcd(a, b);
        ASSERT( gcd == 120 );
    }

    {
        a = -1920;
        b = 1080;
        gcd = cf_get_gcd(a, b);
        ASSERT( gcd == 120 );
    }

    return 0;
}

static int test_case_arithmatics_special(void)
{
    /* sqrt(2)^2 */
    {
        cf * c = cf_create_from_terms_i(30, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                        2, 2, 2, 2, 2, 2, 2, 2, 2);
        cf * c2 = cf_create_from_bihomographic(c, c,
                                               1, 0, 0, 0,
                                               0, 0, 0, 1);

        char * s = cf_convert_to_string_canonical(c2, 10);
        char * s2 = cf_convert_to_string_float(c2, 10);

        printf("\n  sqrt(2)^2 ~ %s ~ %s\n", s, s2);

        cf_free(c2);
        cf_free(c);
        free(s);
        free(s2);
    }

    /* pi / pi */
    {
        cf * pi = cf_create_from_pi();
        cf * one = cf_create_from_bihomographic(pi, pi, 0, 1, 0, 0, 0, 0, 1, 0);
        char * s = cf_convert_to_string_canonical(one, 10);
        char * sf = cf_convert_to_string_float(one, 10);

        printf("  pi / pi ~ %s ~ %s\n", s, sf);

        cf_free(pi);
        cf_free(one);
        free(s);
        free(sf);
    }

    return 0;
}

static int test_case_canonical_float_string(void)
{
    {
        const char * fl = "0";
        char * fl2 = canonical_float_string(fl);

        ASSERT( strcmp(fl2, "0") == 0);

        free(fl2);
    }

    {
        const char * fl = " + 0";
        char * fl2 = canonical_float_string(fl);

        ASSERT( strcmp(fl2, "0") == 0);

        free(fl2);
    }

    {
        const char * fl = " - 0";
        char * fl2 = canonical_float_string(fl);

        ASSERT( strcmp(fl2, "-0") == 0);

        free(fl2);
    }

    {
        const char * fl = " - 10.090 ";
        char * fl2 = canonical_float_string(fl);

        ASSERT( strcmp(fl2, "-10.090") == 0);

        free(fl2);
    }

    return 0;
}

int main(void)
{
    TEST( arithmatics );
    TEST( convergent );
    TEST( calculate_pi_gcf );
    TEST( calculate_pi );
    TEST( convert_from_float );
    TEST( convert_pi_string );
    TEST( convert_string_canonical );
    TEST( convert_string_float );
    TEST( gcd );
    TEST( arithmatics_special );
    TEST( canonical_float_string );

    return 0;
}
