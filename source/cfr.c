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
    fprintf(stderr, "    -m, --maxden=integer    max denominator\n"
                    "        --maxnum=integer    max numerator\n"
                    "    -n, --number=integer    max number of continued fraction coeffs\n"
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
                    "        --sqrt              square root\n"
                    "        --int-bits=bits     integer precision in bits to calculate bihomographic\n"
                    "        --root=m/n          root of {}^{m/n}\n"
                    "    -f  --float=precision   generate float expression\n"
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
    fprintf(stderr, "    %s -n400 --int-bits=1024 --sqrt 3 13\n", name);
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
    int prints_float;
    int find_root; /* 1-sqrt, 2-root */
    int int_bits;
    int index, root_m, root_n;
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
    struct cfstep * step = (struct cfstep*)calloc(1, sizeof(struct cfstep));
    step->t = *t;
    step->gcd = gcd;
    step->prev = NULL;
    step->next = ctx->steps;
    if (ctx->steps)
    {
        ctx->steps->prev = step;
    }
    ctx->steps = step;
    ++ctx->index;
}

static void cfr_ctx_free_steps(struct context* ctx)
{
    struct cfstep * step = ctx->steps;
    ctx->steps = NULL;
    while (step)
    {
        struct cfstep * next = step->next;
        free(step);
        step = next;
    }
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
    fields = (struct range*)calloc(ctx->index + 1, sizeof(struct range));
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
typedef enum {
    False = 0,
    True = 1
} Boolean;

/* check the word is a natrual */
static Boolean check_natrual(const char * word)
{
    int i;
    if (!word) return False;
    for (i = 0; word[i] != '\0'; i++)
    {
        if (word[i] < '0' || word[i] > '9')
            return False;
    }
    return i != 0;
}
static Boolean parse_natrual(const char * word, int * n)
{
    if (check_natrual(word))
    {
        *n = atoi(word);
        return True;
    }
    return False;
}
static Boolean parse_natrual_ll(const char * word, long long * ll)
{
    if (check_natrual(word))
    {
        *ll = atoll(word);
        return True;
    }
    return False;
}
static Boolean parse_fraction(const char * word, int * num, int * den)
{
    int i, nn = 1, mm = 1;
    const char * pm = NULL;
    if (!word)
    {
        return False;
    }
    for (i = 0; word[i] != '\0'; i++)
    {
        if (word[i] < '0' || word[i] > '9')
        {
            if (word[i] == '/' && pm == NULL)
            {
                if (i > 0)
                {
                    nn = atoi(word);
                    pm = word + i + 1;
                }
                else
                {
                    return False;
                }
            }
            else
            {
                return False;
            }
        }
    }
    if (i == 1)
    {
        return False;
    }
    if (pm && pm[0] != '\0')
    {
        mm = atoi(pm);
    }
    *num = nn;
    *den = mm;
    return True;
}

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
            {"sqrt",      no_argument,       0,  0 },
            {"int-bits",  required_argument, 0,  0 },
            {"root",      required_argument, 0,  0 },
            {"float",     required_argument, 0, 'f'},
            {"help",      no_argument,       0, 'h'},
            {"version",   no_argument,       0,  0 },
            {0,           0,                 0,  0 }
        };

        c = getopt_long(argc, argv, "m:n:wprcgslvf:h",
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
                if (!parse_natrual_ll(optarg, &ctx->limits.max_numerator))
                {
                    fprintf(stderr,
                            "Argument Error: Max numerator wants a natrual number, "
                            "but of '%s'.\n", optarg);
                    exit(1);
                }
            }
            else
            if (strcmp(long_options[option_index].name, "sqrt") == 0)
            {
                ctx->find_root = 1;
            }
            else
            if (strcmp(long_options[option_index].name, "int-bits") == 0)
            {
                if (!parse_natrual(optarg, &ctx->int_bits))
                {
                    fprintf(stderr, "Error parsing argument: --int-bits=%s\n", optarg);
                    exit(1);
                }
            }
            else
            if (strcmp(long_options[option_index].name, "root") == 0)
            {
                if (parse_fraction(optarg, &ctx->root_m, &ctx->root_n))
                {
                    if (ctx->root_m > ctx->root_n)
                    {
                        fprintf(stderr, "Error: root of x^{%s} is not supported\n", optarg);
                        exit(1);
                    }
                    ctx->find_root = 2;
                }
                else
                {
                    fprintf(stderr, "Error parsing argument: --root=%s\n", optarg);
                    exit(1);
                }
            }
            break;

        case 'm':
            if (!parse_natrual_ll(optarg, &ctx->limits.max_denominator))
            {
                fprintf(stderr, "Error parsing argument: --maxden=%s\n", optarg);
                exit(1);
            }
            break;

        case 'n':
            if (!parse_natrual(optarg, &ctx->limits.max_index))
            {
                fprintf(stderr, "Error parsing argument: --number=%s", optarg);
                exit(1);
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

        case 'f':
            if (parse_natrual(optarg, &ctx->prints_float))
            {
                ctx->show_mod = c;
            }
            else
            {
                fprintf(stderr, "Error parsing argument: --float=%s\n", optarg);
                exit(1);
            }
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
        nums = (long long *)calloc(size, sizeof(long long));
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
                    if (ctx->int_bits < 64)
                        ctx->x = cf_create_from_bihomographic(cfx, cfy, 0, 1, 0, 0, 0, 0, 1, 0);
                    else
                        ctx->x = cf_create_from_bihomo_pre(cfx, cfy, 0, 1, 0, 0, 0, 0, 1, 0, ctx->int_bits);
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
    ctx.find_root = 0;
    ctx.int_bits = 63;
    ctx.root_m = 1;
    ctx.root_n = 1;
    ctx.prints_float = -1;
    ctx.show_mod = '\0';
    ctx.steps = NULL;

    /* parse options */
    if (parse_options(argc, argv, &ctx) != 0)
    {
        exit(1);
    }

    if (ctx.find_root)
    {
        /* simplify and calculate square root */
        struct context ctx_simp;
        fraction f;
        cf * cfx, *cfy;
        int is_minus = 0;
        memcpy(&ctx_simp, &ctx, sizeof(ctx));
        if (ctx.x)
            ctx_simp.x = cf_copy(ctx.x);
        ctx_simp.show_mod = 's';
        ctx_simp.find_root = 0;
        ctx_simp.steps = NULL;
        if (ctx_simp.is_float)
        {
            if (ctx_simp.den == NULL)
            {
                fraction f = rational_best_for(ctx_simp.num);
                cf_free(ctx_simp.x);
                ctx_simp.x = cf_create_from_fraction(f);
            }
            else
            {
                fraction f1 = rational_best_for(ctx_simp.num);
                fraction f2 = rational_best_for(ctx_simp.den);
                cf * c1 = cf_create_from_fraction(f1);
                cf * c2 = cf_create_from_fraction(f2);
                cf * c;
                if (ctx.int_bits < 64)
                    c  = cf_create_from_bihomographic(c1, c2,
                                                      0, 1, 0, 0,
                                                      0, 0, 1, 0);
                else
                    c  = cf_create_from_bihomo_pre(c1, c2,
                                                   0, 1, 0, 0,
                                                   0, 0, 1, 0, ctx.int_bits);
                cf_free(ctx_simp.x);
                ctx_simp.x = c;
                cf_free(c1);
                cf_free(c2);
            }
        }
        ctx_simp.steps = (struct cfstep*) calloc(1, sizeof(struct cfstep));
        cfr(ctx_simp.x, 0, &ctx_simp.limits, cfrcb_accept_simp, &ctx_simp);
        f = ctx_simp.steps->t.convergent;
        if (f.n < 0)
        {
            is_minus = 1;
            f.n = -f.n;
        }
        cf_free(ctx.x);
        ctx.x = NULL;
        if (ctx.find_root == 1)
            cfx = cf_create_from_sqrt_n(f.n);
        else
            cfx = cf_create_from_nth_root(f.n, ctx.root_n, ctx.root_m);
        if (f.d == 1ll)
        {
            ctx.x = cfx;
        }
        else
        {
            if (ctx.find_root == 1)
                cfy = cf_create_from_sqrt_n(f.d);
            else
                cfy = cf_create_from_nth_root(f.d, ctx.root_n, ctx.root_m);
            if (ctx.int_bits < 64)
                ctx.x = cf_create_from_bihomographic(cfx, cfy, 0, (is_minus? -1 : 1), 0, 0,
                                                     0, 0, 1, 0);
            else
                ctx.x = cf_create_from_bihomo_pre(cfx, cfy, 0, (is_minus? -1 : 1), 0, 0,
                                                  0, 0, 1, 0, ctx.int_bits);
            cf_free(cfx);
            cf_free(cfy);
        }
        ctx.is_float = 0;
        if (ctx.limits.max_index == INT_MAX)
        {
            ctx.limits.max_index = 100;
        }
        if (ctx.prints_float >= 0)
            ctx.show_mod = 'f';
        else
            ctx.show_mod = 'c';
        ctx.is_welformed = 0;
        cf_free(ctx_simp.x);
        ctx_simp.x = NULL;
        cfr_ctx_free_steps(&ctx_simp);
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
        else if (ctx.find_root)
        {
            fprintf(stderr, "calculating gcd from root is unsupported.");
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
                    cf * c;
                    if (ctx.int_bits < 64)
                        c  = cf_create_from_bihomographic(c1, c2,
                                                          0, 1, 0, 0,
                                                          0, 0, 1, 0);
                    else
                        c  = cf_create_from_bihomo_pre(c1, c2,
                                                       0, 1, 0, 0,
                                                       0, 0, 1, 0, ctx.int_bits);
                    cf_free(ctx.x);
                    ctx.x = c;
                    cf_free(c1);
                    cf_free(c2);
                }
            }
            ctx.steps = (struct cfstep*) calloc(1, sizeof(struct cfstep));
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
    case 'f':
        {
            char * fl = cf_convert_to_string_float(ctx.x, ctx.prints_float);
            if (fl)
            {
                printf("%s\n", fl);
                free(fl);
            }
        }
        break;
    case 'r':
        /* no break */
    default:
        gcd = ctx.is_float ? 0 : cf_get_gcd(ctx.rat.n, ctx.rat.d);
        ctx.is_complete = cfr(ctx.x, gcd, &ctx.limits, cfrcb_collect_steps, &ctx);
        print_report(argc, argv, &ctx);
    }
    if (ctx.num)
    {
        free(ctx.num);
    }
    if (ctx.den)
    {
        free(ctx.den);
    }
    cfr_ctx_free_steps(&ctx);
    cf_free(ctx.x);
    return 0;
}

// vim:set fdm=marker:
