#include <stdio.h>
#include "cf.h"

static void test_case1(void)
{
    cf * c;

    printf("case1: -16 / 9\n");

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
    printf("       h = ------\n");
    printf("           0x + 1\n");

    c1 = cf_create_from_fraction((fraction){16, 9});
    c2 = cf_create_from_homography(c1, 1, 0, 0, 1);
    while (!cf_is_finished(c2))
    {
        printf("%lld ", cf_next_term(c2));
    }
    printf("\n");
    cf_free(c2);
    cf_free(c1);
}

static void test_case3(void)
{
    cf * c1, *c2;

    printf("case3: x = 16 / 9\n");
    printf("           -x + 0\n");
    printf("       h = ------\n");
    printf("           0x + 1\n");

    c1 = cf_create_from_fraction((fraction){16, 9});
    c2 = cf_create_from_homography(c1, -1, 0, 0, 1);
    while (!cf_is_finished(c2))
    {
        printf("%lld ", cf_next_term(c2));
    }
    printf("\n");
    cf_free(c2);
    cf_free(c1);
}

static void test_case4(void)
{
    cf * c1, *c2;

    printf("case4: x = 16 / 9\n");
    printf("           0x + 1\n");
    printf("       h = ------\n");
    printf("           1x + 0\n");

    c1 = cf_create_from_fraction((fraction){16, 9});
    c2 = cf_create_from_homography(c1, 0, 1, 1, 0);
    while (!cf_is_finished(c2))
    {
        printf("%lld ", cf_next_term(c2));
    }
    printf("\n");
    cf_free(c2);
    cf_free(c1);
}

static void test_case5(void)
{
    cf * c1, *c2;

    printf("case5: x = 16 / 9\n");
    printf("           0x + 1\n");
    printf("       h = ------\n");
    printf("           -x + 0\n");

    c1 = cf_create_from_fraction((fraction){16, 9});
    c2 = cf_create_from_homography(c1, 0, 1, -1, 0);
    while (!cf_is_finished(c2))
    {
        printf("%lld ", cf_next_term(c2));
    }
    printf("\n");
    cf_free(c2);
    cf_free(c1);
}

static void test_case6(void)
{
    cf * c1, *c2;

    printf("case6: x =-16 / 9\n");
    printf("           0x + 1\n");
    printf("       h = ------\n");
    printf("           -x + 0\n");

    c1 = cf_create_from_fraction((fraction){-16, 9});
    c2 = cf_create_from_homography(c1, 0, 1, -1, 0);
    while (!cf_is_finished(c2))
    {
        printf("%lld ", cf_next_term(c2));
    }
    printf("\n");
    cf_free(c2);
    cf_free(c1);
}

int main(void)
{
    test_case1();
    test_case2();
    test_case3();
    test_case4();
    test_case5();
    test_case6();
    return 0;
}
