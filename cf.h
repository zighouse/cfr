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

struct _cf {
    long long (*next_term)(struct _cf *c);
    int (*is_finished)(struct _cf *c);
    void (*reset)(struct _cf *c);
    void (*free)(struct _cf *c);
};
typedef struct _cf cf;

#define cf_next_term(c)    (c)->next_term(c)
#define cf_is_finished(c)  (c)->is_finished(c)
#define cf_reset(c)        (c)->reset(c)
#define cf_free(c)         (c)->free(c)

cf * cf_create_from_fraction(fraction f);

#endif // __CF_H__
