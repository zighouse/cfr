#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "cf.h"

static void test_case1(void)
{
    cf * c;

    printf("case1: -16 / 9 = ");

    c = cf_create_from_fraction((fraction){-16, 9});
    while (!cf_is_finished(c))
    {
        printf("%lld ", cf_next_term(c));
    }
    printf("\n");
    cf_free(c);
}

static void test_case2(void)
{
    cf * c1, *c2;

    printf("case2: x = 16 / 9\n");
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

static void test_case3(void)
{
    cf * c1, *c2;

    printf("case3: x = 16 / 9\n");
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

static void test_case4(void)
{
    cf * c1, *c2;

    printf("case4: x = 16 / 9\n");
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

static void test_case5(void)
{
    cf * c1, *c2;

    printf("case5: x = 16 / 9\n");
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

static void test_case6(void)
{
    cf * c1, *c2;

    printf("case6: x =-16 / 9\n");
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

static void test_case7(void)
{
    cf * h, *x, *y;
    double fx, fy;
    char *s1, *s2;
    fx = -16.0/9.0;
    fy = 127.0/50.0;

    printf("case7: x =-16 / 9, y = 127 / 50 \n");
    printf("           3xy + 2x +  y + 0\n");
    printf("       h = ----------------- = %f = ",
           (3*fx*fy+2*fx+fy)/(-fx*fy-2*fx-3*fy+4));

    x = cf_create_from_fraction((fraction){-16, 9});
    y = cf_create_from_fraction((fraction){127, 50});
    h = cf_create_from_bihomographic(x, y,
                                      3,  2,  1, 0,
                                     -1, -2, -3, 4);
    s1 = cf_convert_to_string_canonical(h, 20);
    s2 = cf_convert_to_string_float(h, 20);
    while (!cf_is_finished(h))
    {
        printf("%lld ", cf_next_term(h));
    }
    printf("\n");
    printf("           -xy - 2x - 3y + 4\n");
    printf("CF...: %s\n", s1);
    printf("float: %s [FIXME] error if minus.\n", s2);

    cf_free(x);
    cf_free(y);
    cf_free(h);
    free(s1);
    free(s2);
}

static void test_case8(void)
{
    cf * h, *x, *y;
    double fx, fy;
    fx = -16.0/9.0;
    fy = 127.0/50.0;

    printf("case8: x =-16 / 9, y = 127 / 50 \n");
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

static void test_case9(void)
{
    cf * c;
    cf_converg_gen * gen;

    printf("case9: 16 / 9: \n");

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

static void test_case10(void)
{
    cf *c;
    cf_converg_gen *s;
    long long numbers[4] = {1, 1, 3, 2};

    printf("case10: x = 1, 1, 3, 2: \n");
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

static void test_case11(void)
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

    printf("case11: gcf(pi): ");

    g = gcf_create_from_pairs(pairs, sizeof(pairs)/sizeof(pairs[0]));
    c = cf_create_from_ghomo(g, 1, 0, 0, 1);
    while (!cf_is_finished(g))
    {
        number_pair pn = cf_next_term(g);
        printf("(%lld, %lld) ", pn.a, pn.b);
    }
    printf("\n");
    printf("cf(pi): ");
    while (!cf_is_finished(c))
    {
        printf(" %lld", cf_next_term(c));
    }
    printf("\n");

    cf_free(g);
    cf_free(c);
}

static void test_case12(void)
{
    int limit, i;
    gcf *g = gcf_create_from_pi();
    cf * c = cf_create_from_ghomo(g, 1, 0, 0, 1);
    char * s = cf_convert_to_string_canonical(c, 100);

    limit = 10;
    printf("case12: gcf(pi): ");
    for (i = 0; i < limit; ++i)
    {
        number_pair pn = cf_next_term(g);
        printf("(%lld,%lld) ", pn.a, pn.b);
    }
    printf("...\n");

    printf("cf(pi): %s\n", s);

    cf_free(g);
    cf_free(c);
    free(s);
}

static void test_case13(void)
{
    cf  *c = cf_create_from_pi();
    char * s = cf_convert_to_string_float(c, 100);

    printf("case13: pi = %s\n", s);

    cf_free(c);
    free(s);
}

static void test_case14(void)
{
    double pi = 3.141592653589793;
    cf  *c;
    char * str;

    printf("case14: pi = %.15f\n", pi);

    c = cf_create_from_float(pi);
    str = cf_convert_to_string_canonical(c, 20);
    printf("cf(pi): %s\n", str);

    cf_free(c);
    free(str);
}

static void test_case15(void)
{
    const char * pi = "3.141592653589793238462643383279502884197169399";
    cf  *c;
    char * str1, * str2;

    printf("case15: pi = %s\n", pi);

    c = cf_create_from_string_float(pi);
    str1 = cf_convert_to_string_canonical(c, 20);
    str2 = cf_convert_to_string_float(c, 50);
    printf("cf(pi): %s\n", str1);
    printf("dec(pi) = %s\n", str2);

    free(str1);
    free(str2);
    cf_free(c);
}

static void test_case16(void)
{
    long long a, b;
    a = 1920;
    b = 1080;
    printf("case16: gcd(%lld, %lld) = %lld\n", a, b, cf_get_gcd(a, b));
}

static int test_case_convert_string_float(void)
{
    long long numbers[] = {-2};
    cf *c = cf_create_from_terms(numbers, 1);
    char * cf, * fl;
    cf = cf_convert_to_string_canonical(c, 20);
    fl = cf_convert_to_string_float(c, 20);
    printf("cf: %s\n", cf);
    printf("fl: %s\n", fl);
    free(cf);
    free(fl);
    cf_free(c);
    return 0;
}

#define TEST(case_name) \
{ \
    printf("CASE: " #case_name); \
    if (test_case_ ##case_name () == 0) \
        printf("OK\n"); \
    else \
        printf("FAIL\n"); \
}

int main(void)
{
    test_case1();
    test_case2();
    test_case3();
    test_case4();
    test_case5();
    test_case6();
    test_case7();
    test_case8();
    test_case9();
    test_case10();
    test_case11();
    test_case12();
    test_case13();
    test_case14();
    test_case15();
    test_case16();

    TEST(convert_string_float);
    return 0;
}
