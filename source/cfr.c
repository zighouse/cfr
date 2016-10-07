/*
** calculate continued fraction for a fraction or a real number.
** In long long integers by Xie Zhigang, 27 Aug 2016
** With corrections from Arno Formella, May 2008 (http://www.ics.uci.edu/~eppstein/numth/frap.c)
** David Eppstein / UC Irvine / 8 Aug 1993
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
#include <getopt.h>

#define VERSION "1.0"

struct _fraction {
    long long n;
    long long d;
};
struct _settings {
    long long max_denominator;
    long long max_numerator;
    int max_index;
};
typedef struct _fraction fraction;
typedef struct _settings settings;

/*
 * inform that after an iteration, the continued fraction coefficient ai is calculated.
 * f   - fraction at current iteration.
 * ai  - continued fraction coefficient.
 * gcd - gcd or 0 if is approximation.
 * data - opaque user data
 */
typedef void(*cfrcb)(fraction f, long long ai, long long gcd, void * data);

static int cfr(fraction f, settings *limits, cfrcb print, void * data)
{
    long long m[4];
    long long ai;
    int index;

    m[0] = m[3] = 1ll;
    m[2] = m[1] = 0ll;
    index = 0;

    while (f.d)
    {
        fraction t;
        long long mod;

        ai = f.n / f.d;
        mod = f.n % f.d;
        if (ai < 0)
        {
            --ai;
            mod += f.d;
        }

        t.n = m[0] * ai + m[2];
        t.d = m[1] * ai + m[3];

        if (t.n > limits->max_numerator ||
            t.d > limits->max_denominator ||
            ++index >= limits->max_index)
        {
            return 0; /* not complete */
        }
            
        print(t, ai, mod ? 0 : f.d, data);

        m[2] = m[0];
        m[3] = m[1];

        m[0] = t.n;
        m[1] = t.d;

        f.n = f.d;
        f.d = mod;
    }

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
                    "    -l, --list              list iteration process\n"
                    "\n"
                    "    -h, --help              display this help\n"
                    "    --version               output version information\n"
                    "    --                      following minus number not parsed as option(s)\n");

    fprintf(stderr, "\nExample:\n");
    fprintf(stderr, "    %s 144 89\n", name);
    fprintf(stderr, "    %s -c 89 144\n", name);
    fprintf(stderr, "    %s -c -p 55 144\n", name);
    fprintf(stderr, "    %s -c -n 4 3.1419265\n", name);
    fprintf(stderr, "    %s -s -m 200 3.1419265\n", name);
    fprintf(stderr, "    %s -c -p -n 18 2.71828182845964\n", name);
    fprintf(stderr, "    %s -l 1920 1080\n", name);
    fprintf(stderr, "    %s -cp -- -16 9\n", name);
    fprintf(stderr, "\nReport bugs to: http://github.com/zighouse/cfr/issues .\n");
}

struct cfstep {
    fraction simp;
    long long ai;
    long long gcd;
    struct cfstep * next;
    struct cfstep * prev;
};

struct context {
    fraction f;
    double x;
    settings limits;
    //int sign;
    int is_float;
    int is_welformed;
    int is_complete;
    int index;
    char show_mod; /* continued, gcd, simple, verbose */
    struct cfstep * steps;
};

static void cfrcb_print_verb(fraction f, long long ai, long long gcd, void * data)
{
    struct context * ctx = (struct context*) data;
    // columns: simp, ai, error or gcd
    if (gcd)
    {
        if (ctx->is_float)
        { 
            printf("%lld/%lld %lld err=0\n", f.n, f.d, ai);
        }
        else
        {
            printf("%lld/%lld %lld gcd=%lld\n", f.n, f.d, ai, gcd);
        }
    }
    else
    {
        printf("%lld/%lld %lld err=%g\n", f.n, f.d,
               ai, ctx->x - (double)f.n/(double)f.d);
    }
    ++ctx->index;
}

static void cfrcb_collect_steps(fraction f, long long ai, long long gcd, void * data)
{
    struct context * ctx = (struct context*) data;
    struct cfstep * step = (struct cfstep*)malloc(sizeof(struct cfstep));
    step->simp = f;
    step->ai = ai;
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
    field_len[0] = snprintf(buf, sizeof buf, "%lld", ctx->steps->simp.n);
    field_len[1] = snprintf(buf, sizeof buf, "%lld", ctx->steps->simp.d);
    field_len[2] = snprintf(buf, sizeof buf, "%lld", ctx->steps->ai);
    
    // reorder the list
    head = NULL;
    while (ctx->steps)
    {
        int len;
        struct cfstep * t = ctx->steps;
        ctx->steps = ctx->steps->next;
        t->next = head;
        head = t;
        len = snprintf(buf, sizeof buf, "%lld", t->ai);
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
                snprintf(buf, sizeof buf, "%s", "err = 0");
            }
            else
            {
                snprintf(buf, sizeof buf, "gcd = %lld", head->gcd);
            }
        }
        else
        {
            snprintf(buf, sizeof buf, "err = %e",
                     ctx->x - (double)head->simp.n/(double)head->simp.d);
        }
        printf("%*lld / %-*lld  %*lld  %s\n",
               field_len[0], head->simp.n, 
               field_len[1], head->simp.d,
               field_len[2], head->ai,
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

        field->length = snprintf(buf, sizeof buf, "%lld", step->ai);
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

static void cfrcb_print_gcd(fraction f, long long ai, long long gcd, void * data)
{
    struct context * ctx = (struct context*) data;
    if (gcd && !ctx->is_float)
    {
        printf("%lld\n", gcd);
    }
    ++ctx->index;
}

static void cfrcb_accept_simp(fraction f, long long ai, long long gcd, void * data)
{
    struct context * ctx = (struct context*) data;
    ctx->steps[0].simp = f;
    ++ctx->index;
}

static void cfrcb_print_cont(fraction f, long long ai, long long gcd, void * data)
{
    struct context * ctx = (struct context*) data;
    printf(ctx->index++ ? " %lld" : "%lld", ai);
}

static void print_report(int argc, char ** argv, void *data)
{
    struct context *ctx = (struct context*) data;

    if (!ctx->steps)
    {
        return;
    }

    printf("Input:\n");
    printf("    fraction := %lld / %lld\n", ctx->f.n, ctx->f.d);
    printf("        real := %e (%s)\n", ctx->x, (ctx->is_float ? "float input" : "evaluated"));
    printf("\nConfiguration & limits:\n");
    printf("          sizeof integer := %u\n", (unsigned)sizeof(long long) * 8);
    printf("             max integer := %lld\n", LLONG_MAX);
    printf("           max numerator := %lld\n", ctx->limits.max_numerator);
    printf("         max denominator := %lld\n", ctx->limits.max_denominator);
    printf("    max number of levels := %d\n", ctx->limits.max_index);


    ctx->is_welformed = 1;

    printf("\nResult:\n");
    printf("    Calculation is%s completed.\n", ctx->is_complete ? "" : " not");
    if (ctx->is_complete && !ctx->is_float)
    {
        printf("    GCD = %lld\n", ctx->steps->gcd);
    }
    else
    {
        printf("    GCD is not available.\n");
    }
    printf("    Simple fraction = %lld / %lld\n", ctx->steps->simp.n, ctx->steps->simp.d);

    printf("\nContinued fraction:\n");
    print_welformed_cont(ctx);

    printf("\nCalculating iterations: %d\n", ctx->index);
    print_welformed(ctx);
}

// {{{ parse options
static int parsef(char * arg, fraction *f)
{
    long long v, m;

    do {
        char * endptr;
        v = strtoll(arg, &endptr, 10);
        m = 1;

        if (v == LLONG_MAX || v == LLONG_MIN)
        {
            goto bail;
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
                    goto bail;
                }

                if (endptr && *endptr)
                {
                    goto bail;
                }

                for (; len > 0; --len)
                {
                    m *= 10;
                }
                break;
            }
            goto bail;
        }
    } while (0);

    f->n = v;
    f->d = m;
    return 1;
bail:
    fprintf(stderr, "parse number failed: %s\n", arg);
    return 0;
}

static int parse_options(int argc, char ** argv, struct context *ctx)
{
    int c, sign;

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
            {"help",      no_argument,       0, 'h'},
            {"version",   no_argument,       0,  0 },
            {0,           0,                 0,  0 }
        };

        c = getopt_long(argc, argv, "m:n:wprcgslh",
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

    // parse numerator
    if (optind >= argc)
    {
        return 1;
    }
    if (!strchr(argv[optind], '/'))
    {
        if (!parsef(argv[optind++], &ctx->f))
        {
            return 1;
        }
    }
    else
    {
        char * p = strchr(argv[optind], '/');
        *p++ = '\0';
        if (!parsef(argv[optind], &ctx->f))
        {
            return 1;
        }
        argv[optind] = p;
    }
    if (ctx->f.n < 0)
    {
        ctx->f.n = -ctx->f.n;
        sign = 1;
    }
    else
    {
        sign = 0;
    }
    ctx->is_float = ctx->f.d > 1;

    // parse denominator
    if (optind < argc)
    {
        fraction f;

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

        if (!parsef(argv[optind++], &f))
        {
            return 1;
        }
        if (f.d > 1)
        {
            ctx->is_float = 1;
        }
        if (f.n < 0)
        {
            f.n = -f.n;
            sign = !sign;
        }
        if (ctx->f.d > f.d)
        {
            ctx->f.d = f.n * ctx->f.d / f.d;
        }
        else
        {
            ctx->f.n = ctx->f.n * f.d / ctx->f.d;
            ctx->f.d = f.n;
        }
        if (sign)
        {
            ctx->f.n = -ctx->f.n;
        }
    }
    return 0;
}
// }}}

int main(int argc, char ** argv)
{
    struct context ctx;

    /* default limits */
    memset(&ctx, 0, sizeof(ctx));
    ctx.limits.max_numerator = ctx.limits.max_denominator = LLONG_MAX;
    ctx.limits.max_index = INT_MAX;
    ctx.is_welformed = 1;
    ctx.show_mod = 'r';

    /* parse options */
    if (parse_options(argc, argv, &ctx) != 0)
    {
        exit(1);
    }

    /* calculate float value for latter use */
    ctx.x = (double)ctx.f.n/(double)ctx.f.d;

    switch (ctx.show_mod)
    {
    case 'g':
        /* gcd is conflict with float */
        if (ctx.is_float)
        {
            fprintf(stderr, "calculating gcd from float numbers is unsupported.");
            exit(1);
        }
        cfr(ctx.f, &ctx.limits, cfrcb_print_gcd, &ctx);
        break;
    case 's':
        /* show simple */
        {
            fraction f;
            ctx.steps = (struct cfstep*) malloc(sizeof(struct cfstep));
            cfr(ctx.f, &ctx.limits, cfrcb_accept_simp, &ctx);
            f = ctx.steps->simp;
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
            ctx.is_complete = cfr(ctx.f, &ctx.limits, cfrcb_collect_steps, &ctx);
            print_welformed_cont(&ctx);
        }
        else
        {
            cfr(ctx.f, &ctx.limits, cfrcb_print_cont, &ctx);
            printf("\n");
        }
        break;
        /* list iteration */
    case 'l':
        if (ctx.is_welformed)
        {
            cfr(ctx.f, &ctx.limits, cfrcb_collect_steps, &ctx);
            print_welformed(&ctx);
        }
        else
        {
            cfr(ctx.f, &ctx.limits, cfrcb_print_verb, &ctx);
        }
        break;
    default:
        ctx.is_complete = cfr(ctx.f, &ctx.limits, cfrcb_collect_steps, &ctx);
        print_report(argc, argv, &ctx);
    }
    return 0;
}

// vim:set fdm=marker:
