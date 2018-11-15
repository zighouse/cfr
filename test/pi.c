#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "cf.h"

static void usage(const char * progname)
{
    printf("USAGE: pi [digits]\n");
    printf("compute pi to any specified digits, using continue fraction algorithm.\n");
    printf("-- xie zhigang, 2017-06-09\n");
}

int main(int argc, char ** argv)
{
    int digits = 100;
    cf  *c;
    char * s;
    if (argc > 1)
    {
        if ( strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
        {
            usage(argv[0]);
            return 0;
        }
        else if (argv[1][0] >= '0' && argv[1][0] <= '9')
        {
            digits = atoi(argv[1]);
        }
    }

    printf("calculate pi to %d digits...:\n", digits);

    c = cf_create_from_pi();
    s = cf_convert_to_string_float(c, digits);
    printf("%s\n", s);

    cf_free(c);
    free(s);

    return 0;
}
