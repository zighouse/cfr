#include <stdio.h>
#include "cf.h"
#include "gcf.h"

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
    fx = -16.0/9.0;
    fy = 127.0/50.0;

    printf("case7: x =-16 / 9, y = 127 / 50 \n");
    printf("           3xy + 2x +  y + 0\n");
    printf("       h = ----------------- = %f = ", (3*fx*fy+2*fx+fy)/(-fx*fy-2*fx-3*fy+4));

    x = cf_create_from_fraction((fraction){-16, 9});
    y = cf_create_from_fraction((fraction){127, 50});
    h = cf_create_from_bihomographic(x, y,
                                      3,  2,  1, 0,
                                     -1, -2, -3, 4);
    while (!cf_is_finished(h))
    {
        printf("%lld ", cf_next_term(h));
    }
    printf("\n");
    printf("           -xy - 2x - 3y + 4\n");
    cf_free(x);
    cf_free(y);
    cf_free(h);
}

static void test_case8(void)
{
    cf * h, *x, *y;
    double fx, fy;
    fx = -16.0/9.0;
    fy = 127.0/50.0;

    printf("case8: x =-16 / 9, y = 127 / 50 \n");
    printf("           3xy + 2x +  y + 4\n");
    printf("       h = ----------------- = %f = ", (3*fx*fy+2*fx+fy+4)/(fx*fy+2*fx+3*fy));
  
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
    long long m[4];

    printf("case9: 16 / 9: \n");

    m[0] = m[3] = 1ll;
    m[2] = m[1] = 0ll;

    c = cf_create_from_fraction((fraction){16, 9});
    while (!cf_is_finished(c))
    {
        long long ai;
        fraction s;

        ai = cf_next_term(c);
        s.n = m[0] * ai + m[2];
        s.d = m[1] * ai + m[3];

        m[2] = m[0];
        m[3] = m[1];

        m[0] = s.n;
        m[1] = s.d;
        printf("       %lld, %lld / %lld\n", ai, s.n, s.d);
    }
    cf_free(c);
}

static void test_case10(void)
{
    cf *c;
    cf_simplifier *s;
    long long numbers[4] = {1, 1, 3, 2};

    printf("case10: x = 1, 1, 3, 2: \n");
    printf("\tcf\tapprox\terror = |x-simple|\n");

    //c = cf_create_from_fraction((fraction){16, 9});
    c = cf_create_from_numbers(numbers, sizeof(numbers)/sizeof(numbers[0]));
    s = cf_simplifier_create(c);

    while (!cf_simplifier_is_finished(s))
    {
        error_range e;
        fraction f;
        long long n;
        if (cf_simplifier_next(s, &n, &f, &e))
        {
            if (e.up_bind.n == 0)
            {
                printf("\t%lld\t%lld/%lld\t0\n", n, f.n, f.d);
            }
            else
            {
                printf("\t%lld\t%lld/%lld\t(%lld/%lld,%lld/%lld)\n", n, f.n, f.d,
                       e.low_bind.n, e.low_bind.d,
                       e.up_bind.n, e.up_bind.d);
            }
        }
    }

    cf_free(c);
    cf_simplifier_free(s);
}

static void test_case11(void)
{
    gcf *g;
    number_pair pairs[6] = {
        {  0, 3},
        {1*1, 6},
        {3*3, 6},
        {5*5, 6},
        {7*7, 6},
        {9*9, 6}
    };

    printf("case11: gcf(pi): \n");

    g = gcf_create_from_pairs(pairs, sizeof(pairs)/sizeof(pairs[0]));
    while (!cf_is_finished(g))
    {
        number_pair pn = cf_next_term(g);
        printf("%lld, %lld\n", pn.a, pn.b);
    }

    cf_free(g);
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
    return 0;
}
