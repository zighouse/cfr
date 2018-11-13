/**
 * Definitions of integer and related operations.
 *
 * \author xiezhigang
 * \date 2018-10-27
 */
#ifndef __INTEGER_H__
#define __INTEGER_H__

#include <stdbool.h>
#include <stdint.h>
#include <gmp.h>

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct _integer {
    mpz_t value;
    uint32_t precision;
    bool infinite;
    bool overflow;
} _integer_struct;

typedef _integer_struct integer_t[1];
typedef _integer_struct *integer_ptr;

void integer_config_precision ( uint32_t precision );
uint32_t integer_get_config_precision ( void );

static inline
void integer_init2_with_int32(integer_t n, int32_t value, uint32_t precision)
{
    n->precision = precision;
    n->infinite = false;
    n->overflow = false;
    mpz_init_set_si(n->value, value);
}

static inline
void integer_init2_with_uint32(integer_t n, uint32_t value, uint32_t precision)
{
    n->precision = precision;
    n->infinite = false;
    n->overflow = false;
    mpz_init_set_ui(n->value, value);
}

static inline
void integer_init2_with_int64(integer_t n, int64_t value, uint32_t precision)
{
    n->precision = precision;
    n->infinite = false;
    n->overflow = false;
    mpz_init(n->value);
    if (value < 0)
    {
        value = -value;
        mpz_import(n->value, 1, -1, sizeof(value), 0, 0, &value);
        mpz_neg(n->value, n->value);
    }
    else
    {
        mpz_import(n->value, 1, -1, sizeof(value), 0, 0, &value);
    }
}

static inline
void integer_init2_with_uint64(integer_t n, uint64_t value, uint32_t precision)
{
    n->precision = precision;
    n->infinite = false;
    n->overflow = false;
    mpz_init(n->value);
    mpz_import(n->value, 1, -1, sizeof(value), 0, 0, &value);
}

static inline 
void integer_init_with_int32(integer_t n, int32_t value)
{
    integer_init2_with_int32(n, value, integer_get_config_precision());
}

static inline
void integer_init_with_uint32(integer_t n, uint32_t value)
{
    integer_init2_with_uint32(n, value, integer_get_config_precision());
}

static inline
void integer_init_with_int64(integer_t n, int64_t value)
{
    integer_init2_with_int64(n, value, integer_get_config_precision());
}

static inline
void integer_init_with_uint64(integer_t n, uint64_t value)
{
    integer_init2_with_uint64(n, value, integer_get_config_precision());
}

static inline
void integer_init (integer_t n)
{
    integer_init_with_int32(n, 0);
}

static inline
void integer_init2 (integer_t n, unsigned precision)
{
    integer_init2_with_int32(n, 0, precision);
}

static inline
void integer_clear ( integer_t n )
{
    mpz_clear(n->value);
}

void integer_inits (integer_ptr, ...);
void integer_inits_pre (uint32_t precision, integer_ptr, ...);
void integer_clears ( integer_ptr, ...);

static inline
bool integer_is_infinite ( integer_t n )
{
    return n->infinite;
}

static inline
bool integer_is_overflow ( integer_t n )
{
    return n->overflow || mpz_sizeinbase(n->value, 2) > n->precision;
}

static inline
bool integer_is_zero ( integer_t n )
{
    return mpz_sgn(n->value) == 0;
}

static inline
int integer_cmp ( integer_t a, integer_t b )
{
    if (a->infinite)
        return 2;
    else if (integer_is_overflow(a))
        return 2;
    else if (b->infinite)
        return -2;
    else if (integer_is_overflow(b))
        return -2;
    else
    {
        int i;
        i = mpz_cmp(a->value, b->value);
        return i > 0 ? 1 : i < 0 ? -1 : 0;
    }
}

static inline
bool integer_equals ( integer_t a, integer_t b )
{
    return integer_cmp( a, b ) == 0;
}

static inline
int32_t integer_sgn ( integer_t n )
{
    return mpz_sgn(n->value);
}

static inline
void integer_set ( integer_t dest, integer_t a )
{
    dest->infinite = a->infinite;
    dest->overflow = a->overflow;
    mpz_set(dest->value, a->value);
}

static inline
void integer_set_int32 ( integer_t dest, int32_t i )
{
    dest->infinite = false;
    dest->overflow = false;
    mpz_set_si(dest->value, i);
}

static inline
void integer_set_int64(integer_t n, int64_t value)
{
    n->infinite = false;
    n->overflow = false;
    if (value < 0)
    {
        value = -value;
        mpz_import(n->value, 1, -1, sizeof(value), 0, 0, &value);
        mpz_neg(n->value, n->value);
    }
    else
    {
        mpz_import(n->value, 1, -1, sizeof(value), 0, 0, &value);
    }
}

static inline
void integer_set_uint64(integer_t n, uint64_t value)
{
    n->infinite = false;
    n->overflow = false;
    mpz_import(n->value, 1, -1, sizeof(value), 0, 0, &value);
}

static inline
void integer_neg ( integer_t dest, integer_t a )
{
    dest->infinite = a->infinite;
    dest->overflow = integer_is_overflow(a);
    if (!dest->infinite)
        mpz_neg(dest->value, a->value);
}

static inline
void integer_add ( integer_t dest, integer_t a, integer_t b )
{
    dest->infinite = a->infinite || b->infinite;
    dest->overflow = integer_is_overflow(a) || integer_is_overflow(b);
    if (!dest->infinite && !dest->overflow)
        mpz_add(dest->value, a->value, b->value);
}

static inline
void integer_sub ( integer_t dest, integer_t a, integer_t b )
{
    dest->infinite = a->infinite || b->infinite;
    dest->overflow = integer_is_overflow(a) || integer_is_overflow(b);
    if (!dest->infinite && !dest->overflow)
        mpz_sub(dest->value, a->value, b->value);
}

static inline
void integer_mul ( integer_t dest, integer_t a, integer_t b )
{
    dest->infinite = a->infinite || b->infinite;
    dest->overflow = integer_is_overflow(a) || integer_is_overflow(b);
    if (!dest->infinite && !dest->overflow)
        mpz_mul(dest->value, a->value, b->value);
}

static inline
void integer_div ( integer_t dest, integer_t a, integer_t b )
{
    dest->infinite = a->infinite || b->infinite || mpz_sgn(b->value) == 0;
    dest->overflow = integer_is_overflow(a) || integer_is_overflow(b);
    if (!dest->infinite && !dest->overflow)
        mpz_div(dest->value, a->value, b->value);
}

static inline
void integer_add_int32 ( integer_t dest, integer_t a, int32_t i )
{
    dest->infinite = a->infinite;
    dest->overflow = integer_is_overflow(a);
    if (!dest->infinite && !dest->overflow)
    {
        if (i > 0)
            mpz_add_ui(dest->value, a->value, (uint32_t)i);
        else if (i < 0)
            mpz_sub_ui(dest->value, a->value, (uint32_t)(-i));
        else
            mpz_set(dest->value, a->value);
    }
}

static inline
void integer_sub_int32 ( integer_t dest, integer_t a, int32_t i )
{
    dest->infinite = a->infinite;
    dest->overflow = integer_is_overflow(a);
    if (!dest->infinite && !dest->overflow)
    {
        if (i > 0)
            mpz_sub_ui(dest->value, a->value, (uint32_t)i);
        else if (i < 0)
            mpz_add_ui(dest->value, a->value, (uint32_t)(-i));
        else
            mpz_set(dest->value, a->value);
    }
}

static inline
void integer_inc ( integer_t dest, int32_t i )
{
    mpz_t x;
    mpz_init(x);
    if (!dest->infinite && !dest->overflow)
    {
        if (i >= 0)
        {
            mpz_add_ui(x, dest->value, (uint32_t)i);
        }
        else if (i < 0)
        {
            mpz_sub_ui(x, dest->value, (uint32_t)(-i));
        }
        mpz_set(dest->value, x);
    }
    mpz_clear(x);
}

static inline
void integer_dec ( integer_t dest, int32_t i )
{
    mpz_t x;
    mpz_init(x);
    if (!dest->infinite && !dest->overflow)
    {
        if (i >= 0)
        {
            mpz_sub_ui(x, dest->value, (uint32_t)i);
        }
        else if (i < 0)
        {
            mpz_add_ui(x, dest->value, (uint32_t)(-i));
        }
        mpz_set(dest->value, x);
    }
    mpz_clear(x);
}

static inline
void integer_mul_int32 ( integer_t dest, integer_t a, int32_t i )
{
    dest->infinite = a->infinite;
    dest->overflow = integer_is_overflow(a);
    if (!dest->infinite && !dest->overflow)
        mpz_mul_si(dest->value, a->value, i);
}

static inline
void integer_div_int32 ( integer_t dest, integer_t a, int32_t i )
{
    dest->infinite = a->infinite || i == 0;
    dest->overflow = integer_is_overflow(a);
    if (!dest->infinite && !dest->overflow)
    {
        if (i > 0)
            mpz_div_ui(dest->value, a->value, (uint32_t)i);
        else
            mpz_div_ui(dest->value, a->value, (uint32_t)(-i));
    }
}

static inline
int64_t integer_get_int64 ( integer_t n )
{
    uint64_t result = 0;
    size_t bytes;
    if (n->infinite || integer_is_overflow(n))
    {
        if (mpz_sgn(n->value) >= 0)
            return LLONG_MAX;
        else
            return LLONG_MIN;
    }
    switch (mpz_sgn(n->value))
    {
    case 0:
        return 0ll;
    case -1:
        bytes = (mpz_sizeinbase(n->value, 16) + 1) / 2;
        if (sizeof(result) > bytes)
        {
            mpz_export(&result, 0, -1, sizeof(result), 0, 0, n->value);
            return -result;
        }
        else
            return LLONG_MIN;
    default:
        bytes = (mpz_sizeinbase(n->value, 16) + 1) / 2;
        if (sizeof(result) > bytes)
        {
            mpz_export(&result, 0, -1, sizeof(result), 0, 0, n->value);
            return result;
        }
        else return LLONG_MAX;
    }
}

static inline
uint64_t integer_get_uint64 ( integer_t n )
{
    uint64_t result = 0;
    size_t bytes;
    if (n->infinite || integer_is_overflow(n))
    {
        return ULLONG_MAX;
    }
    bytes = (mpz_sizeinbase(n->value, 16) + 1) / 2;
    if (sizeof(result) > bytes)
    {
        mpz_export(&result, 0, -1, sizeof(result), 0, 0, n->value);
        return result;
    }
    else
    {
        return ULLONG_MAX;
    }
}

static inline
void integer_diff ( integer_t dest, integer_t a, integer_t b )
{
    dest->infinite = a->infinite || b->infinite;
    dest->overflow = integer_is_overflow(a) || integer_is_overflow(b);
    if (!dest->infinite && !dest->overflow)
    {
        mpz_t x;
        mpz_init(x);
        mpz_sub(x, a->value, b->value);
        mpz_abs(dest->value, x);
        mpz_clear(x);
    }
}

static inline
integer_ptr integer_max ( integer_t a, integer_t b)
{
    int r = integer_cmp(a, b);
    if (r >= 0)
        return a;
    else
        return b;
}

#if defined (__cplusplus)
}
#endif

#endif // __INTEGER_H__
