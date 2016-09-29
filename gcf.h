#ifndef __GCF_H__
#define __GCF_H__

/*
 * generalized continued fraction
 *
 *                     a1
 * gcf = b0 + -----------------------
 *                        a2
 *            b1 + ------------------
 *                           a3
 *                 b2 + -------------
 *                              a4
 *                      b3 + --------
 *                           b4 + ...
 * number_pair-0:  0, b0
 * number_pair-1: a1, b1
 * number_pair-2: a2, b2
 * number_pair-3: a3, b3
 * number_pair-4: a4, b4
 * number_pair-5:  1, oo (truncated)
 */
typedef struct _gcf gcf;
typedef struct _gcf_class gcf_class;
typedef struct _number_pair number_pair;

struct _number_pair {
    long long a;
    long long b;
};

struct _gcf_class {
    number_pair (*next_term)(gcf *g);
    int (*is_finished)(const gcf * const g);
    void (*free)(gcf *g);
    gcf * (*copy)(const gcf * const g);
};

struct _gcf {
    gcf_class * object_class;
};

gcf * gcf_create_from_pairs(const number_pair * const arr, unsigned int size);
#endif // __GCF_H__
