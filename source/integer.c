#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <gmp.h>

#include "integer.h"

static uint32_t config_precision = 64;

void integer_config_precision ( uint32_t precision )
{
    config_precision = precision;
}

uint32_t integer_get_config_precision ( void )
{
    return config_precision;
}

void integer_inits(integer_ptr x, ...)
{
    va_list ap;
    va_start (ap, x);
    while (x != NULL)
    {
        integer_init(x);
        x = va_arg(ap, integer_ptr);
    }
    va_end(ap);
}

void integer_inits_pre (uint32_t precision, integer_ptr x, ...)
{
    va_list ap;
    va_start (ap, x);
    while (x != NULL)
    {
        integer_init2(x, precision);
        x = va_arg(ap, integer_ptr);
    }
    va_end(ap);
}

void integer_clears ( integer_ptr x, ... )
{
    va_list ap;
    va_start (ap, x);
    while (x != NULL)
    {
        integer_clear(x);
        x = va_arg(ap, integer_ptr);
    }
    va_end(ap);
}
