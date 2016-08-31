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
#include <unistd.h>

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

static fraction cfr(fraction f, long long maxnum, long long maxden, cfrcb print, void * data)
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
            break;
        }

        m[2] = m[0];
        m[3] = m[1];

        m[0] = t.n;
        m[1] = t.d;

        print(t, ai, mod ? 0 : f.d, data);

        f.n = f.d;
        f.d = mod;
    }

    return (fraction){m[0], m[1]};
}

static void help(char * name)
{
    int main(int argc, char ** argv);
    char * argv[5];
    int fd;

    argv[0] = name;
    fprintf(stderr, "Lists a serial of simple fractions approximate original fraction or real,\n"
                    "and continued fraction coefficients, errors or gcd.  "
                    "Version: %s\n\n", VERSION);

    fprintf(stderr, "SYNOPSIS\n");
    fprintf(stderr, "    %s [options] num [den]\n", name);

    fprintf(stderr, "\nOPTIONS\n"
                    "    num    - is real number , or numerator of a fraction\n\n"
                    "    den    - [optional] is the denominator, integer or another real\n\n"
                    "    -m integer|-minteger|--maxden=integer\n"
                    "                      specify the maximum denominator\n\n"
                    "    --maxnum=integer  specify the maximum numerator\n\n"
                    "    -w|--welformed    print in welformed style\n\n"
                    "    -p|--plain        print in plain style\n\n"
                    "    -h|--help         show help\n\n"
                    "    --version         show version\n\n");

    fd = dup(1);
    dup2(2, 1); /* redirect stdout to stderr */

    fprintf(stderr, "\nEXAMPLE\n\n");

    fprintf(stderr, "$ %s -m 200 -w 3.14159\n", name);
    argv[1] = "-m";
    argv[2] = "200";
    argv[3] = "-w";
    argv[4] = "3.14159";
    main(5, argv);
    fprintf(stderr, "result shows the best fraction with den<=200 approx 3.14159 is 355/113,\n");
    fprintf(stderr, "and the continued fraction is: 3 + 1/(7 + 1/(15 + 1/(1 + ...))).\n");
    fprintf(stderr, "\n");

    fprintf(stderr, "$ %s -w 1920 1080\n", name);
    argv[1] = "-w";
    argv[2] = "1920";
    argv[3] = "1080";
    main(4, argv);
    fprintf(stderr, "result shows: 1920/1080 = 16/9, and the gcd of 1920 and 1080 is 120.\n");
    fprintf(stderr, "\n");

    fprintf(stderr, "AUTHOR\n"
                    "    Written by Xie Zhigang, 27 Aug 2016, based on version of:\n"
                    "        Arno Formella, May 2008\n"
                    "        David Eppstein / UC Irvine / 8 Aug 1993\n");

    dup2(fd, 1); /* restore stdout redirection */
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
    int index;
    struct cfstep * steps;
};

static void print_result(fraction f, long long ai, long long gcd, void * data)
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

static void on_step(fraction f, long long ai, long long gcd, void * data)
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
    sign = ctx->sign ? " - " : " ";
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
                        printf("%s\n", VERSION);
                        exit (0);
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
    if (!parsef(argv[optind++], &ctx->f))
    {
        return 1;
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
    struct context ctx = {0};

    /* default settings */
    ctx.maxnum = ctx.maxden = LLONG_MAX;
    ctx.is_welformed = 1;

    /* parse options */
    if (parse_options(argc, argv, &ctx) != 0)
    {
        exit(1);
    }

    /* calculate float value for latter use */
    ctx.x = (double)ctx.f.n/(double)ctx.f.d;

    if (ctx.is_welformed)
    {
        cfr(ctx.f, ctx.maxnum, ctx.maxden, on_step, &ctx);
        print_welformed(&ctx);
    }
    else
    {
        cfr(ctx.f, ctx.maxnum, ctx.maxden, print_result, &ctx);
    }

    return 0;
}

// vim:set fdm=marker:
