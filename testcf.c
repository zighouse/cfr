#include <stdio.h>
#include "cf.h"

int main(void)
{
    cf * c = cf_create_from_fraction((fraction){-16, 9});
    while (!cf_is_finished(c))
    {
        printf("%lld ", cf_next_term(c));
    }
    printf("\n");
    cf_free(c);
    return 0;
}
