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

cf * cf_create_from_fraction(fraction f);
cf * cf_create_from_homography(const cf * const x,
                               long long a, long long b,
                               long long c, long long d);
#endif // __CF_H__
