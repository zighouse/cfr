#include <stdio.h>
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
    return 0;
}
