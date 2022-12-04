#include "bignum.h"

struct bignum_pair {
    struct bignum first;
    struct bignum second;
};

struct goldschmidt_pair {
    struct bignum first;
    struct bignum second;
    size_t decimalPlaces;
    bool direction;
};