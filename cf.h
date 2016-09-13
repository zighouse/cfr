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

struct _cf;
struct _cf_class {
    long long (*next_term)(struct _cf *c);
    int (*is_finished)(struct _cf *c);
    void (*free)(struct _cf *c);
};
struct _cf {
    struct _cf_class * object_class;
};
typedef struct _cf_class cf_class;
typedef struct _cf cf;

#define cf_class(c)        (c)->object_class
#define cf_next_term(c)    cf_class(c)->next_term(c)
#define cf_is_finished(c)  cf_class(c)->is_finished(c)
#define cf_free(c)         cf_class(c)->free(c)

cf * cf_create_from_fraction(fraction f);

#endif // __CF_H__
