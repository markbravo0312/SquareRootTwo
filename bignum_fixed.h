#pragma once

#include "bignum.h"

struct bignum_fixed {
    struct bignum *bignum;
    size_t comma; // TODO: Discuss which alignment the comma should follow
};

char *toDec(struct bignum_fixed b);

char *toHexComma(struct bignum_fixed bn);

char *convertWholeBignumToDecimal(struct bignum wholeNumber);