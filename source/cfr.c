/*
 * Calculate continued fraction for a fraction or a real number.
 * 
 * -- Xie Zhigang, 2016-10-08
 */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "cf.h"

#define VERSION "0.1.1"

struct _settings {
    long long max_denominator;
    long long max_numerator;
    int max_index;
};
typedef struct _settings settings;

/*
 * inform that after an iteration, the continued fraction coefficient ai is calculated.
 * t   - term at current iteration.
 * gcd - gcd or 0 if is approximation.
 * data - opaque user data
 */
typedef void(*cfrcb)(cf_converg_term * t, long long gcd, void * data);

static int cfr(cf * x, long long gcd, settings *limits, cfrcb print, void * data)
{
    cf_converg_gen * cfap = cf_converg_gen_create(x);
    cf_converg_term term;
    int index = 0;

    while (!cf_is_finished(cfap))
    {
        term = cf_next_term(cfap);
        if (term.convergent.n > limits->max_numerator ||
            term.convergent.d > limits->max_denominator ||
            ++index > limits->max_index)
        {
            cf_free(cfap);
            return 0; /* not complete */
        }
        if (cf_is_finished(cfap))
        {
            print(&term, gcd, data);
        }
        else
        {
            print(&term, 0, data);
        }
    }

    cf_free(cfap);
    return 1; /* complete */
}

static void help(char * name)
{
    fprintf(stderr, "Usage: %s [OPTIONS] NUMERATOR [[/] DENOMINATOR]\n", name);
    fprintf(stderr, "    -m, --maxden=integer    maximum denominator\n"
                    "        --maxnum=integer    maximum numerator\n"
                    "    -n, --number=integer    maximum number of contined fraction coeffs\n"
                    "\n"
                    "    -w, --welformed         welformed style\n"
                    "    -p, --plain             plain style\n"
                    "\n"
                    "    -r, --report            report result\n"
                    "    -c, --cont              display continued fraction\n"
                    "    -g, --gcd               display gcd of two integers\n"
                    "    -s, --simple            display approximate simple fraction\n"
                    "                            print the best rational for float in round convention\n"
                    "    -l, --list              list iteration process\n"
                    "\n"
                    "    -v, --reverse           convert a continued fraction into fraction\n"
                    "\n"
                    "    -h, --help              display this help\n"
                    "    --version               output version information\n"
                    "    --                      following minus number not parsed as option(s)\n");

    fprintf(stderr, "\nExample:\n");
    fprintf(stderr, "    %s 144 89\n", name);
    fprintf(stderr, "    %s -c 89 144\n", name);
    fprintf(stderr, "    %s -c -p 55 144\n", name);
    fprintf(stderr, "    %s -c -n 4 3.14159\n", name);
    fprintf(stderr, "    %s -s 3.142\n", name);
    fprintf(stderr, "    %s -s 3.1416\n", name);
    fprintf(stderr, "    %s -rv 3 7 16\n", name);
    fprintf(stderr, "    %s -c -p -n 18 2.71828182845964\n", name);
    fprintf(stderr, "    %s -l 1920 1080\n", name);
    fprintf(stderr, "    %s -cp -- -16 9\n", name);
    fprintf(stderr, "\nReport bugs to: http://github.com/zighouse/cfr/issues .\n");
}

struct cfstep {
    cf_converg_term t;
    long long gcd;
    struct cfstep * next;
    struct cfstep * prev;
};

struct context {
    cf * x;
    fraction rat;
    settings limits;
    int is_reverse;
    int is_float;
    int is_welformed;
    int is_complete;
    int index;
    char show_mod; /* continued, gcd, simple, verbose */
    struct cfstep * steps;
    char * num, * den;
};

static void cfrcb_print_verb(cf_converg_term *t, long long gcd, void * data)
{
    struct context * ctx = (struct context*) data;
    // columns: simp, ai, error or gcd
    if (gcd)
    {
        if (ctx->is_float)
        { 
            printf("%lld/%lld %lld (err = 0)\n",
                   t->convergent.n, t->convergent.d, t->coef);
        }
        else
        {
            printf("%lld/%lld %lld gcd=%lld\n",
                   t->convergent.n, t->convergent.d, t->coef, gcd);
        }
    }
    else
    {
        printf("%lld/%lld %lld (1/%lld < err < 1/%lld)\n",
               t->convergent.n, t->convergent.d, t->coef,
               t->lower_error, t->upper_error);
    }
    ++ctx->index;
}

static void cfrcb_collect_steps(cf_converg_term *t, long long gcd, void * data)
{
    struct context * ctx = (struct context*) data;
    struct cfstep * step = (struct cfstep*)malloc(sizeof(struct cfstep));
    step->t = *t;
    step->gcd = gcd;
    step->next = ctx->steps;
    if (ctx->steps)
    {
        ctx->steps->prev = step;
    }
    ctx->steps = step;
    ++ctx->index;
}

// print welformed list
static void print_welformed(void * data)
{
    struct context * ctx = (struct context*) data;
    struct cfstep * head;
    int field_len[3];
    char buf[100];

    if (!ctx->steps)
    {
        return;
    }

    // get max field length
    field_len[0] = snprintf(buf, sizeof buf, "%lld", ctx->steps->t.convergent.n);
    field_len[1] = snprintf(buf, sizeof buf, "%lld", ctx->steps->t.convergent.d);
    field_len[2] = snprintf(buf, sizeof buf, "%lld", ctx->steps->t.coef);
    
    // reorder the list
    head = NULL;
    while (ctx->steps)
    {
        int len;
        struct cfstep * t = ctx->steps;
        ctx->steps = ctx->steps->next;
        t->next = head;
        head = t;
        len = snprintf(buf, sizeof buf, "%lld", t->t.coef);
        field_len[2] = field_len[2] > len ? field_len[2] : len;
    }
    ctx->steps = head;

    // print welformed list
    for (head = ctx->steps; head; head = head->next)
    {
        if (head->gcd)
        {
            if (ctx->is_float)
            { 
                snprintf(buf, sizeof buf, "%s", "(err = 0)");
            }
            else
            {
                snprintf(buf, sizeof buf, "(gcd = %lld)", head->gcd);
            }
        }
        else
        {
            snprintf(buf, sizeof buf, "(1/%lld < err < 1/%lld)",
                     head->t.lower_error, head->t.upper_error);
        }
        printf("%*lld / %-*lld  %*lld  %s\n",
               field_len[0], head->t.convergent.n, 
               field_len[1], head->t.convergent.d,
               field_len[2], head->t.coef,
               buf);
    }
}

struct range
{
    int offset;
    char *text;
    int length;
};

static void print_welformed_cont(void *data)
{
    struct context *ctx = (struct context*) data;
    struct cfstep *step;
    struct range *fields, *field, *field_end;

    if (!ctx->steps)
    {
        return;
    }

    /* collect range fields of denominators */
    fields = (struct range*)malloc(sizeof(struct range) * (ctx->index + 1));
    fields[ctx->index].offset = 0;
    fields[ctx->index].text = strdup("...");
    fields[ctx->index].length = 3;
    field = &fields[ctx->index - 1];
    field_end = &fields[ctx->is_complete ? ctx->index : ctx->index + 1];
    for (step = ctx->steps; step; step = step->next)
    {
        struct range * p;
        char buf[100];

        field->length = snprintf(buf, sizeof buf, "%lld", step->t.coef);
        field->text = strdup(buf);
        field->offset = 0;

        for (p = field + 1; p < field_end; ++p)
        {
            p->offset += field->length + 4;
        }
        --field;
    }

    /* expand the range fields into lines */
    for (field = fields; field < field_end; ++field)
    {
        int line_start, line_end;
        if (field < field_end - 1)
        {
            int i;
            line_start = field[1].offset - 1;
            line_end = field_end[-1].offset + field_end[-1].length + 1;
            printf("%*d\n", (line_end + line_start) / 2 + 1, 1);
            printf("%*s + ", field->offset + field->length, field->text);
            for (i = line_start; i < line_end; ++i)
            {
                putc('-', stdout);
            }
            putc('\n', stdout);
        }
        else
        {
            printf("%*s\n", field->offset + field->length, field->text);
        }
        free(field->text);
    }
    free(fields);
}

static void cfrcb_accept_simp(cf_converg_term *t, long long gcd, void * data)
{
    struct context * ctx = (struct context*) data;
    ctx->steps[0].t = *t;
    ++ctx->index;
}

static void cfrcb_print_cont(cf_converg_term *t, long long gcd, void * data)
{
    struct context * ctx = (struct context*) data;
    printf(ctx->index++ ? " %lld" : "%lld", t->coef);
}

static void print_report(int argc, char ** argv, void *data)
{
    struct context *ctx = (struct context*) data;

    if (!ctx->steps)
    {
        return;
    }

    ctx->is_welformed = 1;

    printf("\nStatus:\n");
    printf("    Calculation is%s completed.\n", ctx->is_complete ? "" : " not");
    if (ctx->is_complete && !ctx->is_float && !ctx->is_reverse)
    {
        printf("    GCD = %lld\n", ctx->steps->gcd);
    }
    else
    {
        printf("    GCD is not available.\n");
    }
    printf("    Simple fraction = %lld / %lld\n",
           ctx->steps->t.convergent.n, ctx->steps->t.convergent.d);

    printf("\nContinued fraction:\n");
    print_welformed_cont(ctx);

    printf("\nCalculating iterations: %d\n", ctx->index);
    print_welformed(ctx);
}

// {{{ parse options
static int parse_options(int argc, char ** argv, struct context *ctx)
{
    char c;
    ctx->num = NULL;
    ctx->den = NULL;

    // parse named options
    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
            {"maxden",    required_argument, 0, 'm'},
            {"maxnum",    required_argument, 0,  0 },
            {"number",    required_argument, 0, 'n'},
            {"welformed", no_argument,       0, 'w'},
            {"plain",     no_argument,       0, 'p'},
            {"report",    no_argument,       0, 'r'},
            {"cont",      no_argument,       0, 'c'},
            {"gcd",       no_argument,       0, 'g'},
            {"simple",    no_argument,       0, 's'},
            {"list",      no_argument,       0, 'l'},
            {"reverse",   no_argument,       0, 'v'},
            {"help",      no_argument,       0, 'h'},
            {"version",   no_argument,       0,  0 },
            {0,           0,                 0,  0 }
        };

        c = getopt_long(argc, argv, "m:n:wprcgslvh",
                        long_options, &option_index);
        if (c == -1)
        {
            break;
        }

        switch (c) {
        case 0:
            if (strcmp(long_options[option_index].name, "version") == 0)
            {
                printf("%s\n", VERSION);
                exit(0);
            }
            else
            if (strcmp(long_options[option_index].name, "maxnum") == 0)
            {
                int n = atoll(optarg);
                if (n > 0)
                {
                    ctx->limits.max_numerator = n;
                }
            }
            break;

        case 'm':
            {
                long long n = atoll(optarg);
                if (n > 0)
                {
                    ctx->limits.max_denominator = n;
                }
            }
            break;

        case 'n':
            {
                long long i = atoi(optarg);
                if (i > 0)
                {
                    ctx->limits.max_index = i;
                }
            }
            break;

        case 'w':
            ctx->is_welformed = 1;
            break;

        case 'p':
            ctx->is_welformed = 0;
            break;

        case 'r':
        case 'c':
        case 'g':
        case 's':
        case 'l':
            ctx->show_mod = c;
            break;

        case 'v':
            /* reverse, calculate rational from a continued fraction. */
            ctx->is_reverse = 1;
            break;

        case 'h':
            help(argv[0]);
            exit(0);
            break;

        default:
            printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

    if (argc == 1)
    {
        help(argv[0]);
        exit(0);
    }

    // parse numerator and/or other operands
    if (optind >= argc)
    {
        return 1;
    }
    if (ctx->is_reverse)
    {
        /* reverse function directly and exit */
        long long *nums;
        int size = argc - optind, i;
        nums = (long long *)malloc(sizeof(long long) * (size));
        if (!nums)
        {
            fprintf(stderr, "no memory\n");
            return 1;
        }
        for (i = 0; i < size; i++)
        {
            nums[i] = atoll(argv[i + optind]);
        }
        ctx->x = cf_create_from_terms(nums, size);
        if (!ctx->show_mod)
        {
            ctx->show_mod = 's';
        }
        free(nums);
        return 0;
    }
    ctx->num = strdup(argv[optind++]);
    {
        char * p = strrchr(ctx->num, '/');
        if (p)
        {
            *p = '\0';
            ++p;
            if (*p)
            {
                ctx->den = strdup(p);
            }
        }
    }

    // parse denominator
    if (!ctx->den && optind < argc)
    {
        if (*argv[optind] == '/')
        {
            if (argv[optind][1] == '\0')
            {
                optind++;
                if (optind == argc)
                {
                    fprintf(stderr, "no denominator\n");
                    return 1;
                }
            }
            else
            {
                argv[optind]++;
            }
        }

        ctx->den = strdup(argv[optind++]);
    }
    {
        cf *cfx = NULL, *cfy = NULL;
        long long nx = 0ll, ny = 0ll;

        ctx->is_float = 0;
        if (strchr(ctx->num, '.'))
        {
            cfx = cf_create_from_string_float(ctx->num);
            ctx->is_float = 1;
        }
        else
        {
            nx = atoll(ctx->num);
        }
        if (ctx->den)
        {
            if (strchr(ctx->den, '.'))
            {
                cfy = cf_create_from_string_float(ctx->den);
                if (ctx->is_float)
                {
                    ctx->x = cf_create_from_bihomographic(cfx, cfy, 0, 1, 0, 0, 0, 0, 1, 0);
                    cf_free(cfx);
                    cf_free(cfy);
                }
                else
                {
                    ctx->is_float = 1;
                    ctx->x = cf_create_from_homographic(cfy, 0, nx, 1, 0);
                    cf_free(cfy);
                }
            }
            else
            {
                ny = atoll(ctx->den);
                if (ctx->is_float)
                {
                    ctx->x = cf_create_from_homographic(cfx, 1, 0, 0, ny);
                    cf_free(cfx);
                }
                else
                {
                    ctx->x = cf_create_from_fraction((fraction){nx, ny});
                    ctx->rat = (fraction){nx, ny};
                }
            }
        }
        else
        {
            if (ctx->is_float)
            {
                ctx->x = cfx;
            }
            else
            {
                ctx->x =cf_create_from_fraction((fraction){nx, 1ll});
                ctx->rat = (fraction){nx, 1ll};
            }
        }
    }
    return 0;
}
// }}}

int main(int argc, char ** argv)
{
    struct context ctx;
    long long gcd;

    /* default limits */
    memset(&ctx, 0, sizeof(ctx));
    ctx.limits.max_numerator = ctx.limits.max_denominator = LLONG_MAX;
    ctx.limits.max_index = INT_MAX;
    ctx.is_welformed = 1;
    ctx.show_mod = '\0';

    /* parse options */
    if (parse_options(argc, argv, &ctx) != 0)
    {
        exit(1);
    }

    switch (ctx.show_mod)
    {
    case 'g':
        /* gcd is conflict with float */
        if (ctx.is_float)
        {
            fprintf(stderr, "calculating gcd from float numbers is unsupported.");
            exit(1);
        }
        gcd = cf_get_gcd(ctx.rat.n, ctx.rat.d);
        printf("%lld\n", gcd);
        break;
    case 's':
        /* show simple */
        {
            fraction f;
            if (ctx.is_float)
            {
                if (ctx.den == NULL)
                {
                    fraction f = rational_best_for(ctx.num);
                    cf_free(ctx.x);
                    ctx.x = cf_create_from_fraction(f);
                }
                else
                {
                    fraction f1 = rational_best_for(ctx.num);
                    fraction f2 = rational_best_for(ctx.den);
                    cf * c1 = cf_create_from_fraction(f1);
                    cf * c2 = cf_create_from_fraction(f2);
                    cf * c  = cf_create_from_bihomographic(c1, c2,
                                                           0, 1, 0, 0,
                                                           0, 0, 1, 0);
                    cf_free(ctx.x);
                    ctx.x = c;
                    cf_free(c1);
                    cf_free(c2);
                }
            }
            ctx.steps = (struct cfstep*) malloc(sizeof(struct cfstep));
            cfr(ctx.x, 0, &ctx.limits, cfrcb_accept_simp, &ctx);
            f = ctx.steps->t.convergent;
            if (ctx.is_welformed)
            {
                printf("%lld / %lld\n", f.n, f.d);
            }
            else
            {
                printf("%lld/%lld\n", f.n, f.d);
            }
        }
        break;
    case 'c':
        /* show continued fraction */
        if (ctx.is_welformed)
        {
            ctx.is_complete = cfr(ctx.x, 0, &ctx.limits, cfrcb_collect_steps, &ctx);
            print_welformed_cont(&ctx);
        }
        else
        {
            cfr(ctx.x, 0, &ctx.limits, cfrcb_print_cont, &ctx);
            printf("\n");
        }
        break;
        /* list iteration */
    case 'l':
        gcd = ctx.is_float ? 0 : cf_get_gcd(ctx.rat.n, ctx.rat.d);
        if (ctx.is_welformed)
        {
            cfr(ctx.x, gcd, &ctx.limits, cfrcb_collect_steps, &ctx);
            print_welformed(&ctx);
        }
        else
        {
            cfr(ctx.x, gcd, &ctx.limits, cfrcb_print_verb, &ctx);
        }
        break;
    case 'r':
        /* no break */
    default:
        gcd = ctx.is_float ? 0 : cf_get_gcd(ctx.rat.n, ctx.rat.d);
        ctx.is_complete = cfr(ctx.x, gcd, &ctx.limits, cfrcb_collect_steps, &ctx);
        print_report(argc, argv, &ctx);
    }
    return 0;
}

// vim:set fdm=marker:
