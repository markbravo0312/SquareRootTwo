#pragma once

#include "bignum.h"

struct bignum_fixed {
    struct bignum *bignum;
    size_t comma; 
};

char *toDec(struct bignum_fixed b);

char *toHexComma(struct bignum_fixed bn);

char *convertWholeBignumToDecimal(struct bignum wholeNumber);
