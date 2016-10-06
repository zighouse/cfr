#include "common.h"

void mpz_set_ull (mpz_t z, unsigned long long ull)
{
    mpz_import(z, 1, -1, sizeof ull, 0, 0, &ull);
}

unsigned long long mpz_get_ull(mpz_t z)
{
    unsigned long long result = 0;
    mpz_export(&result, 0, -1, sizeof result, 0, 0, z);
    return result;
}

void mpz_set_ll(mpz_t z, long long sll)
{
    mpz_import(z, 1, -1, sizeof sll, 0, 0, &sll);
    if (sll < 0)
    {
        z->_mp_size = -z->_mp_size;
    }
}

long long mpz_get_ll(mpz_t z)
{
    long long result = 0;
    mpz_export(&result, 0, -1, sizeof result, 0, 0, z);
    return result;
}
