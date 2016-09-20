#ifndef __CF_H__
#define __CF_H__
/*!
 * continued fration
 *
 * \author xiezhigang
 * \date   2016-09-07
 */

struct _fraction {
    long long n;
    long long d;
};
typedef struct _fraction fraction;

typedef struct _cf cf;
typedef struct _cf_class cf_class;

struct _cf_class {
    long long (*next_term)(cf *c);
    int (*is_finished)(const cf * const c);
    void (*free)(cf *c);
    cf * (*copy)(const cf * const c);
};

struct _cf {
    cf_class * object_class;
};

#define cf_class(c)        (c)->object_class
#define cf_next_term(c)    cf_class(c)->next_term(c)
#define cf_is_finished(c)  cf_class(c)->is_finished(c)
#define cf_free(c)         cf_class(c)->free(c)
#define cf_copy(c)         cf_class(c)->copy(c)

/*
 * create a continued fraction from rational:
 *      n
 * f = ---
 *      d
 */
cf * cf_create_from_fraction(fraction f);

/*
 * create a continued fraction from homograhic function:
 *     ax + b
 *     ------
 *     cx + d
 */
cf * cf_create_from_homographic(const cf * const x,
                                long long a, long long b,
                                long long c, long long d);
/*
 * create a continued fraction from bihomograhic function:
 *     axy + bx + cy + d
 *     -----------------
 *     exy + fx + gy + h
 */
cf * cf_create_from_bihomographic(const cf * const x, const cf * const y,
                                  long long a, long long b, long long c, long long d,
                                  long long e, long long f, long long g, long long h);

typedef struct _error_range error_range;
struct _error_range {
    int type; // bit-0: includes low, bit-1: includes up
    fraction low_bind;
    fraction up_bind;
};

typedef struct _cf_simplifier cf_simplifier;
typedef struct _cf_simplifier_class cf_simplifier_class;
struct _cf_simplifier {
    cf *c;
    long long m[4];
    long long ai[2];
    fraction s[2];
    int idx, finished;
};

cf_simplifier * cf_simplifier_create(const cf * const c);
int cf_simplifier_next(cf_simplifier * s, long long *ai, fraction *f, error_range * e);
int cf_simplifier_is_finished(const cf_simplifier * const s);
void cf_simplifier_free(cf_simplifier * s);

#endif // __CF_H__
