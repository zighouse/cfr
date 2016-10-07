/*
 * Definitions of continued fraction and related operations.
 *
 * \author xiezhigang
 * \date   2016-09-07
 */
#ifndef __CF_H__
#define __CF_H__

#if defined (__cplusplus)
extern "C" {
#endif


/*
 * Fraction in long long integers.
 *
 *                  numerator
 *     fraction = ------------- .
 *                 denominator
 *
 * denominator != 0.
 */
typedef struct _fraction fraction;

struct _fraction {
    long long n; /* numerator */
    long long d; /* denominator */
};

/*
 * Continued fraction in long long integers.
 *
 * A continued fraction is defined as following format as wikipedia:
 *
 *                        1
 *     cf = a0 + --------------------
 *                          1
 *               a1 + ---------------
 *                             1
 *                    a2 + ----------
 *                         .
 *                          .      1
 *                           . + ---- .
 *                                an
 *
 * The serials of `a0, a1, a2, ..., an' are terms of a continued
 * fraction, and the type `cf' defines them in long long integers in
 * C-language.
 *
 *     a0 can be zero, negative, or positive.
 *     if i > 0 then ai > 0 .
 *     if ai == oo then is finished or is truncated.
 *
 * There are different implementations according to different algorithm
 * or different inner-structure a continued fraction is depends on.  The
 * type `cf' is only a common part with inner-structure hidden.
 *
 * Application that uses this library should implements its' own
 * inner-structure of one algorithm or use `cf_create_*' functions to
 * create a new continued fraction instance.  And after use of one
 * instance, should free it by using `cf_free()' helper macro.
 */
typedef struct _cf cf;

/*
 * Class definition for a family of continued fractions.
 */
typedef struct _cf_class cf_class;

struct _cf_class {

    /*
     * Retrieve next term from a continued fraction.
     *
     * Any term after retrieved from is not supposed to can be
     * re-retrieved from this continued fraction instance any more.
     */
    long long (*next_term)(cf *c);

    /*
     * Check whether the continued fraction is finished and no more
     * terms can be retrieved from.
     */
    int (*is_finished)(const cf * const c);

    /*
     * Free a continued fraction.
     */
    void (*free)(cf *c);

    /*
     * Make a copy of a continued fraction.
     *
     * The copy is made under the current status, and all terms have
     * been retrieved from the continued fraction are no more
     * retrievable from the new copy.
     *
     * Returns a continued fraction of the same status with c, and
     * should be freed by `void (*free)(cf *c)'.
     */
    cf * (*copy)(const cf * const c);
};

struct _cf {
    /* Class definition of a continued fraction */
    cf_class * object_class;
};

/*
 * Helper macro to get the class definition of a continued fraction.
 */
#define cf_class(c)        (c)->object_class

/*
 * Helper macro to retrieve next term from a continued fraction.
 */
#define cf_next_term(c)    cf_class(c)->next_term(c)

/*
 * Helper macro to check whether the continued fraction is finished and
 * no more terms can be retrieved from.
 */
#define cf_is_finished(c)  cf_class(c)->is_finished(c)

/*
 * Helper macro to free a continued fraction.
 */
#define cf_free(c)         cf_class(c)->free(c)

/*
 * Helper macro to make a copy of a continued fraction.
 *
 * The copy is made under the current status, and all terms have been
 * retrieved from the continued fraction are no more retrievable from
 * the new copy.
 */
#define cf_copy(c)         cf_class(c)->copy(c)

/*
 * Create a continued fraction from terms.
 *
 * It copies a new array of terms.
 *
 *                        1
 *     cf = a0 + --------------------
 *                          1
 *               a1 + ---------------
 *                             1
 *                    a2 + ----------
 *                         .
 *                          .      1
 *                           . + ---- .
 *                                an
 *
 * \param terms: a0, a1, a2, ..., an.
 * \param size: n + 1.
 *
 * Free by `cf_free()' helper macro.
 */
cf * cf_create_from_terms(const long long * const terms,
                          unsigned int size);

/*
 * Create a continued fraction from a fraction, which is the final value
 * of it.
 *
 *           n
 *     cf = ---
 *           d
 *
 * Free by `cf_free()' helper macro.
 */
cf * cf_create_from_fraction(fraction f);

/*
 * Create a continued fraction from homograhic function of another
 * continued fraction.
 *
 *          ax + b
 *     cf = ------
 *          cx + d
 *
 * By using this function, arithmetics of a continued fraction and
 * numbers can be calculated. Given a continued fraction `x' and a
 * number `n':
 *
 * x + n is:
 *
 *          1x + n
 *     cf = ------
 *          0x + 1
 *
 * x - n is:
 *
 *          1x - n
 *     cf = ------
 *          0x + 1
 *
 * n * x is:
 *
 *          nx + 0
 *     cf = ------
 *          0x + 1
 *
 * n / x is:
 *
 *          0x + n
 *     cf = ------
 *          1x + 0
 *
 * x / n is:
 *
 *          1x + 0
 *     cf = ------
 *          0x + n
 *
 * The identity of a continued fraction is a kind of copy, but it has
 * special features such as it out put canonical terms if original one
 * is not a canonical one. A canonical CF have terms of positive
 * integers in its fractional part. Identity of x is:
 *
 *          1x + 0
 *     cf = ------
 *          0x + 1
 *
 * Free by `cf_free()' helper macro.
 */
cf * cf_create_from_homographic(const cf * const x,
                                long long a, long long b,
                                long long c, long long d);

/*
 * Create a continued fraction from bihomograhic function.
 * 
 *          axy + bx + cy + d
 *     cf = -----------------
 *          exy + fx + gy + h
 *
 * By using this function, arithmetics between two continued fractions
 * can be calculated, given two continued fractions `x' and `y':
 *
 * x + y is:
 *
 *          0xy + 1x + 1y + 0
 *     cf = -----------------
 *          0xy + 0x + 0y + 1
 *
 * x - y is:
 *
 *          0xy + 1x - 1y + 0
 *     cf = -----------------
 *          0xy + 0x + 0y + 1
 *
 * x * y is:
 *
 *          1xy + 0x + 0y + 0
 *     cf = -----------------
 *          0xy + 0x + 0y + 1
 *
 * x / y is:
 *
 *          0xy + 1x + 0y + 0
 *     cf = -----------------
 *          0xy + 0x + 1y + 0
 *
 * Free by `cf_free()' helper macro.
 */
cf * cf_create_from_bihomographic(const cf * const x,
                                  const cf * const y,
                                  long long a, long long b,
                                  long long c, long long d,
                                  long long e, long long f,
                                  long long g, long long h);

/*
 * Generalized continued fraction.
 *
 *                        a1
 * gcf = b0 + ------------------------------
 *                           a2
 *            b1 + -------------------------
 *                              a3
 *                 b2 + --------------------
 *                                 a4
 *                      b3 + ---------------
 *                           b4 + .
 *                                 .     an
 *                                  . + ---- .
 *                                       bn 
 *
 * term[0]: { 1, b0} (start, the first term, b0 can be zero)
 * term[1]: {a1, b1}
 * term[2]: {a2, b2}
 * term[3]: {a3, b3}
 * term[4]: {a4, b4}
 * term[5]: { 1, oo} (equivalent to be truncated)
 *
 * Each term of GCF is a pair of numbers which expressed as a partial
 * fraction.
 *
 * There are different implementations of GCF, and the inner-structure
 * is differ. The type `gcf' is only a common part with inner-structure
 * hidden.
 *
 * Application that uses this library should implements its' own
 * inner-structure of one algorithm or use `gcf_create_*' functions to
 * create a new GCF instance. And after use should remember to free it
 * by using `cf_free()' helper macro.
 *
 * Helper macros of `cf_class()', `cf_next_term()', `cf_is_finished()',
 * `cf_free()' and `cf_copy()' also applies to GCF.
 */
typedef struct _gcf gcf;
typedef struct _gcf_class gcf_class;

/*
 * A pair of numbers, is served as a term of partial fraction of GCF.
 */
typedef struct _number_pair number_pair;
struct _number_pair {
    long long a; /* the first number */
    long long b; /* the second number */
};

struct _gcf_class {
    number_pair (*next_term)(gcf *g);
    int (*is_finished)(const gcf * const g);
    void (*free)(gcf *g);
    gcf * (*copy)(const gcf * const g);
};

struct _gcf {
    /* Class definition of a GCF */
    gcf_class * object_class;
};

/*
 * Create a GCF from a serial of terms.
 *
 * Often, the first term's first number is 1.
 * If GCF have no integral part, the first term's second number is 0.
 *
 * Free by `cf_free()' helper macro.
 */
gcf * gcf_create_from_pairs(const number_pair * const arr,
                            unsigned int size);

/*
 * Create a GCF which the value is `pi'.
 *
 * Returns an expression of pi in arctan(1):
 *
 *                        4
 *     pi = 0 + ----------------------
 *                          1
 *              1 + ------------------
 *                             4
 *                   3 + -------------
 *                               9
 *                       5 + ---------
 *                                 16
 *                           7 + -----
 *                                ...
 *
 * There are multiple expression of pi in GCF format. This GCF
 * expression is not the best one.
 *
 * To get serials of continued fraction or decimal digits, this GCF can
 * be used:
 *
 *     const int limit = 100;
 *     int i;
 *     gcf *g = gcf_create_from_pi();
 *     cf * c = cf_create_from_ghomo(g, 1, 0, 0, 1);
 *     cf_gen_dec * gen = cf_gen_dec_create(c);
 *
 *     printf("cf(pi): ");
 *     for (i = 0; i < limit; ++i)
 *     {
 *         printf(" %lld", cf_next_term(c));
 *     }
 *     printf(" ...\n");
 *
 *     printf("pi: ");
 *     for (i = 0; i < limit; ++i)
 *     {
 *         int x = cf_gen_dec_next(gen);
 *         if (i == 0)
 *         {
 *             printf("%d.", x);
 *         }
 *         else
 *         {
 *             printf("%d", x);
 *         }
 *     }
 *     printf("...\n");
 *
 *     cf_free(g);
 *     cf_free(c);
 *     cf_gen_dec_free(gen);
 *  
 * Free by `cf_free()' helper macro.
 */
gcf * gcf_create_from_pi(void);

/*
 * Create a continued fraction from homograhic function by input
 * generalized continued fraction `x':
 *
 *          ax + b
 *     cf = ------
 *          cx + d
 *  
 * Free by `cf_free()' helper macro.
 */
cf * cf_create_from_ghomo(const gcf * const x,
                          long long a, long long b,
                          long long c, long long d);

/*
 * CF generator generates digits from a continued fraction.
 *
 * A continued fraction is a good expression of real numbers, but
 * decimal digits is far more convenient. CF generator is used to
 * generate decimal digits one by one.
 *
 * To generate decimal digits for a CF, should use
 * `cf_gen_create_dec()'.
 */
typedef struct _cf_gen cf_gen;
typedef struct _cf_gen_class cf_gen_class;

struct _cf_gen_class {

    /*
     * Retrieve next term (next digit) from a CF generator.
     */
    int (*next_term)(cf_gen *gen);

    /*
     * Check whether the CF generator is finished.
     */
    int (*is_finished)(const cf_gen * const gen);

    /*
     * Free a CF generator.
     */
    void (*free)(cf_gen * gen);

    /*
     * Make a copy of a CF generator for decimal.
     *
     * Returns a new CF generator for decimal, and should be freed by
     * `cf_free()' helper macro.
     */
    cf_gen * (*copy)(const cf_gen * const gen);
};

struct _cf_gen {
    /* Class definition of a continued fraction digits generator */
    cf_gen_class * object_class;
};

/*
 * Create a CF digit generator for decimal.
 *
 * Free by `cf_free()' helper macro.
 */
cf_gen * cf_gen_create_dec(const cf * const c);

/*
 * Rational approximation of continued fraction
 */
typedef struct _cf_approx cf_approx;
typedef struct _cf_approx_class cf_approx_class;

/*
 * Term of rational approximation of CF.
 */
typedef struct _cf_approx_term {
    long long ai;           /* CF term */
    fraction  rational;     /* rational approximation on this term */
                            /* error range of approximation:
                             * numerator is always 1.
                             * If denominator is LLONG_MAX, error is 0.
                             */
    long long lower_error;  /* denominator of lower error */
    long long upper_error;  /* denominator of upper error */
} cf_approx_term;

struct _cf_approx_class {

    /*
     * Retrieve next approximation term
     */
    cf_approx_term (*next_term)(cf_approx * approx);

    /*
     * Check whether the approximation is finished.
     * If is finished, the CF is a rational number.
     */
    int (*is_finished)(const cf_approx * const approx);

    /*
     * Free the approximation
     */
    void (*free)(cf_approx * approx);

    /*
     * Make a new copy of this approximation
     */
    cf_approx * (*copy)(const cf_approx * const approx);
};

struct _cf_approx {
    /* Class definition of a rational approximation of CF */
    cf_approx_class * object_class;
};

/*
 * Create a rational approximation of CF.
 *
 * Free by `cf_free()' helper macro.
 */
cf_approx * cf_approx_create(const cf * const c);


#if defined (__cplusplus)
}
#endif

#endif // __CF_H__
/* vim:set tw=72: */
