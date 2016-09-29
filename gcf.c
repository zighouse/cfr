#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "cf.h"
#include "gcf.h"

static gcf_class _pnumbers_class;

typedef struct _pnumbers pnumbers;
struct _pnumbers {
    gcf base;
    number_pair *arr;
    unsigned int size;
    unsigned int idx;
};

static number_pair pnumbers_next_term(gcf *g)
{
    pnumbers * n = (pnumbers*)g;
    return n->idx < n->size ?
        n->arr[n->idx++] : (number_pair){LLONG_MAX, 1ll};
}

static int pnumbers_is_finished(const gcf * const g)
{
    return ((pnumbers*)g)->idx >= ((pnumbers*)g)->size;
}

static void pnumbers_free(gcf *g)
{
    free(((pnumbers*)g)->arr);
    free(g);
}

static gcf * pnumbers_copy(const gcf * const g)
{
    pnumbers * n = (pnumbers*)g;
    return gcf_create_from_pairs(n->arr + n->idx, n->size - n->idx);
}

static gcf_class _pnumbers_class = {
    pnumbers_next_term,
    pnumbers_is_finished,
    pnumbers_free,
    pnumbers_copy
};

gcf * gcf_create_from_pairs(const number_pair * const arr, unsigned int size)
{
    pnumbers * n;

    if (!size || !arr)
        return NULL;

    n = (pnumbers*)malloc(sizeof(pnumbers));
    if (!n)
        return NULL;

    n->arr = (number_pair*)malloc(size * sizeof(number_pair));
    memcpy(n->arr, arr, size * sizeof(number_pair));
    n->idx = 0;
    n->size = size;
    n->base.object_class = &_pnumbers_class;
    return &n->base;
}

