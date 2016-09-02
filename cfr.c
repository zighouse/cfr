/*
** calculate continued fraction for a fraction or a real number.
** In long long integers by Xie Zhigang, 27 Aug 2016
** With corrections from Arno Formella, May 2008 (http://www.ics.uci.edu/~eppstein/numth/frap.c)
** David Eppstein / UC Irvine / 8 Aug 1993
**
** usage: cfr num [den] [-m maxden]
**   num    is real number to approx, or numerator, integer or float
**   den    [optional] is the denominator, integer or float
**   maxden [optional] is the maximum denominator allowed
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

#define VERSION "1.0"

struct _fraction {
    long long n;
    long long d;
};
typedef struct _fraction fraction;

/*
 * inform that after an iteration, the continued fraction coefficient ai is calculated.
 * f   - fraction at current iteration.
 * ai  - continued fraction coefficient.
 * gcd - gcd or 0 if is approximation.
 * data - opaque user data
 */
typedef void(*cfrcb)(fraction f, long long ai, long long gcd, void * data);

static int cfr(fraction f, long long maxnum, long long maxden, cfrcb print, void * data)
{
    long long m[4];
    long long ai;

    m[0] = m[3] = 1ll;
    m[2] = m[1] = 0ll;

    while (f.d)
    {
        fraction t;
        long long mod;

        ai = f.n / f.d;
        mod = f.n % f.d;

        t.n = m[0] * ai + m[2];
        t.d = m[1] * ai + m[3];

        if (t.n > maxnum || t.d > maxden)
        {
            return 0; /* not complete */
        }

        m[2] = m[0];
        m[3] = m[1];

        m[0] = t.n;
        m[1] = t.d;

        print(t, ai, mod ? 0 : f.d, data);

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
                    "    --version               output version information\n");

    fprintf(stderr, "\nExample:\n");
    fprintf(stderr, "    %s 144 89\n", name);
    fprintf(stderr, "    %s -c -w -m 200 3.1416\n", name);
    fprintf(stderr, "    %s -c -p 2.71828182845964\n", name);
    fprintf(stderr, "    %s -l 1920 1080\n", name);
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
    long long maxnum;
    long long maxden;
    int sign;
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
            printf("%s%lld/%lld %lld err=0\n",
                   (ctx->sign ? "- " : ""), f.n, f.d, ai);
        }
        else
        {
            printf("%s%lld/%lld %lld gcd=%lld\n",
                   (ctx->sign ? "- " : ""), f.n, f.d, ai, gcd);
        }
    }
    else
    {
        printf("%s%lld/%lld %lld err=%e\n",
               (ctx->sign ? "- " : ""), f.n, f.d,
               ai, ctx->x - (double)f.n/(double)f.d);
    }
}

static void cfrcb_collect_steps(fraction f, long long ai, long long gcd, void * data)
{
    struct context * ctx = (struct context*) data;
    struct cfstep * step = (struct cfstep*)malloc(sizeof(struct cfstep));
    ++ctx->index;
    step->simp = f;
    step->ai = ai;
    step->gcd = gcd;
    step->next = ctx->steps;
    if (ctx->steps)
    {
        ctx->steps->prev = step;
    }
    ctx->steps = step;
}

// print welformed list
static void print_welformed(void * data)
{
    struct context * ctx = (struct context*) data;
    struct cfstep * head;
    int field_len[3];
    char buf[100], *sign;

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
    sign = (char*)(ctx->sign ? " - " : " ");
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
        printf("%s%*lld / %-*lld  %*lld  %s\n",
               sign, field_len[0], head->simp.n, 
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
}

static void cfrcb_accept_simp(fraction f, long long ai, long long gcd, void * data)
{
    fraction *output = (fraction *)data;
    *output = f;
}

static void cfrcb_print_cont(fraction f, long long ai, long long gcd, void * data)
{
    struct context * ctx = (struct context*) data;
    if (ctx->index++)
    {
        printf(" %lld", ai);
    }
    else
    {
        printf("%s%lld", ctx->sign ? "(-) " : "", ai);
    }
}

static void print_report(int argc, char ** argv, void *data)
{
    struct context *ctx = (struct context*) data;
    const char * sign;

    if (!ctx->steps)
    {
        return;
    }

    sign = ctx->sign ? "- " : "";

    printf("Input:\n");
    printf("    fraction := %s%lld / %lld\n", sign, ctx->f.n, ctx->f.d);
    printf("        real := %s%e (%s)\n", sign, ctx->x, (ctx->is_float ? "float input" : "evaluated"));
    printf("\nConfiguration & limits:\n");
    printf("     sizeof integer := %lu\n", sizeof(long long) * 8);
    printf("        max integer := %lld\n", LLONG_MAX);
    printf("      max numerator := %lld\n", ctx->maxnum);
    printf("    max denominator := %lld\n", ctx->maxden);

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
    printf("    Simple fraction = %s%lld / %lld\n", sign, ctx->steps->simp.n, ctx->steps->simp.d);

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
    int optind, i;
    // parse named options
    optind = 1;
    if (argc == 1)
    {
        help(argv[0]);
        exit (0);
    }
    for (i = argc; i > 1; --i)
    {
        char * optarg;
        int parsed = 0;
        // parse maxden
        if (argv[optind][0] == '-')
        {
            switch (argv[optind][1])
            {
            case 'c':
            case 'g':
            case 's':
            case 'r':
            case 'l':
                ctx->show_mod = argv[optind][1];
                ++optind; parsed = 1;
                break;
            case 'h':
                help(argv[0]);
                exit(0);
            case 'm':
                {
                    long long md = 0;
                    if (argv[optind][2] != '\0')
                    {
                        optarg = &argv[optind][2];
                    }
                    else if (optind < argc - 1)
                    {
                        optarg = argv[++optind];
                        --i;
                    }
                    else
                    {
                        return 1;
                    }

                    md = atoll(optarg);
                    if (md > 0)
                    {
                        ctx->maxden = md;
                    }
                    else
                    {
                        return 1;
                    }
                    ++optind; parsed = 1;
                }
                break;
            case 'w':
                {
                    ctx->is_welformed = 1;
                    ++optind;
                    parsed = 1;
                }
                break;
            case 'p':
                {
                    ctx->is_welformed = 0;
                    ++optind; parsed = 1;
                }
                break;
            case '-':
                {
                    switch (argv[optind][2])
                    {
                    case 'c':
                    case 'g':
                    case 's':
                    case 'r':
                    case 'l':
                        ctx->show_mod = argv[optind][2];
                        ++optind; parsed = 1;
                        break;
                    case 'h':
                        help(argv[0]);
                        exit(0);
                    case 'p':
                        ctx->is_welformed = 0;
                        ++optind; parsed = 1;
                        break;
                    case 'w':
                        ctx->is_welformed = 1;
                        ++optind; parsed = 1;
                        break;
                    case 'v':
                        ++optind; parsed = 1;
                        if (strstr(argv[optind], "version"))
                        {
                            printf("%s\n", VERSION);
                            exit (0);
                        }
                        break;
                    case 'm':
                        {
                            long long m;
                            if (strstr(&argv[optind][2], "maxnum=") == &argv[optind][2])
                            {
                                optarg = &argv[optind][9];
                                m = atoll(optarg);
                                if (m > 0)
                                {
                                    ctx->maxnum = m;
                                    ++optind; parsed = 1;
                                }
                                break;
                            }
                            else if (strstr(&argv[optind][2], "maxden=") == &argv[optind][2])
                            {
                                optarg = &argv[optind][9];
                                m = atoll(optarg);
                                if (m > 0)
                                {
                                    ctx->maxden = m;
                                    ++optind; parsed = 1;
                                }
                                break;
                            }
                            return 0;
                        }
                    default:
                        break;
                    }
                }
                break;
            default:
                break;
            }
        }
        if (!parsed)
        {
            // push this non-parsed to end
            int j;
            char * arg = argv[optind];
            for (j = optind + 1; j < argc; ++j)
            {
                argv[j - 1] = argv[j];
            }
            argv[j - 1] = arg;
        }
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
        ctx->sign = 1;
    }
    else
    {
        ctx->sign = 0;
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
            ctx->sign = !ctx->sign;
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
    }
    return 0;
}
// }}}

int main(int argc, char ** argv)
{
    struct context ctx;

    /* default settings */
    memset(&ctx, 0, sizeof(ctx));
    ctx.maxnum = ctx.maxden = LLONG_MAX;
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
        cfr(ctx.f, ctx.maxnum, ctx.maxden, cfrcb_print_gcd, &ctx);
        break;
    case 's':
        /* show simple */
        {
            fraction f;
            cfr(ctx.f, ctx.maxnum, ctx.maxden, cfrcb_accept_simp, &f);
            if (ctx.is_welformed)
            {
                printf("%s%lld / %lld\n", ctx.sign ? "- " : "", f.n, f.d);
            }
            else
            {
                printf("%s%lld/%lld\n", ctx.sign ? "-" : "", f.n, f.d);
            }
        }
        break;
    case 'c':
        /* show continued fraction */
        if (ctx.is_welformed)
        {
            ctx.is_complete = cfr(ctx.f, ctx.maxnum, ctx.maxden, cfrcb_collect_steps, &ctx);
            print_welformed_cont(&ctx);
        }
        else
        {
            cfr(ctx.f, ctx.maxnum, ctx.maxden, cfrcb_print_cont, &ctx);
            printf("\n");
        }
        break;
        /* list iteration */
    case 'l':
        if (ctx.is_welformed)
        {
            cfr(ctx.f, ctx.maxnum, ctx.maxden, cfrcb_collect_steps, &ctx);
            print_welformed(&ctx);
        }
        else
        {
            cfr(ctx.f, ctx.maxnum, ctx.maxden, cfrcb_print_verb, &ctx);
        }
        break;
    default:
        ctx.is_complete = cfr(ctx.f, ctx.maxnum, ctx.maxden, cfrcb_collect_steps, &ctx);
        print_report(argc, argv, &ctx);
    }
    return 0;
}

// vim:set fdm=marker:
