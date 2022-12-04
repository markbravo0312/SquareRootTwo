#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "bignum_fixed.h"

size_t startHalfByteDegree(struct bignum x) {
    size_t deg = 0;
    for (size_t i = 0; i < x.size; ++i) {
        uint32_t f = x.data[i];

        for (int j = 0; j < 8; ++j) {
            if ((f & (0xF << j * 4)) != 0)
                return deg;
            deg++;
        }
    }
    return deg;
}

struct bignum alignDecimalPlaces(struct bignum l, size_t comma_places) {
    struct bignum ten = bignumFromInt64(10);
    struct bignum tx = fastExponential(ten, comma_places);
    shiftLeftInPlace(&tx, (comma_places) * 8);

    struct bignum f = copy(l);

    while (startHalfByteDegree(f) <= comma_places) {
        struct bignum r = multiplication(f, tx);
        bignumFree(f);
        f = r;
        shiftRightInPlace(&f, comma_places * 8);
    }

    shiftRightInPlace(&f, comma_places * 4);
    bignumFree(ten);
    bignumFree(tx);

    return f;
}

void alignBignumTo128bit(struct bignum *bn) {
    if (bn->size % 4 == 0)
        return;

    size_t m = bn->size + 4 - (bn->size % 4);
    assert(m > bn->size);
    bn->data = realloc(bn->data, m * sizeof(uint32_t));
    memset(&bn->data[bn->size], 0, (m - bn->size) * 4);
    bn->size = m;
}

// convert a number without decimal places to its decimal string representation
// uses a specialised version of the long division
char *convertWholeBignumToDecimal(struct bignum wholeNumber) {
    assert(wholeNumber.size % 4 == 0);

    const uint64_t dividingVariable = (uint64_t) powl(10, 16);
    size_t size_64 = wholeNumber.size / 2;

    uint128_t *outputNumber = calloc(wholeNumber.size, sizeof(uint128_t));
    if (outputNumber == NULL) {
        printf("calloc failed\n");
        abort();
    }


    uint128_t currentOutputSize = 1;

    size_t i, j;
    // first split down the number into multiple parts of a long decimal string with carries already precalculated,
    // so it can be easily copied via snprintf
    for (i = 0; i < size_64; i++) {
        uint128_t carry = (uint128_t) (((uint64_t *) wholeNumber.data)[size_64 - 1 - i]);

        for (j = 0; j < currentOutputSize; j++) {
            uint128_t cursor = (outputNumber[j] << 64) + carry;
            outputNumber[j] = cursor % dividingVariable;
            carry = cursor / dividingVariable;
        }

        // work down the carry if there is still some left after divisions
        while (carry / dividingVariable != 0) {
            outputNumber[currentOutputSize++] = carry % dividingVariable;
            carry = carry / dividingVariable;
        }

        // push the additional carry
        outputNumber[currentOutputSize++] = carry;
    }

    char *returnString = calloc(currentOutputSize * 16 + 1, 1);
    if (returnString == NULL) {
        printf("calloc failed\n");
        abort();
    }

    char buffer[17] = {0};
    bool top = true;
    for (i = 0; i < currentOutputSize; i++) {
        uint128_t v = outputNumber[currentOutputSize - 1 - i];
        if (v == 0 && i != currentOutputSize - 1) {
            if (top) continue;
        } else {
            if (top) top = false;
        }

        sprintf(buffer, "%016llu", v); // noinspection
        strncat(returnString, buffer, 17);
    }

    free(outputNumber);

    size_t k;
    size_t le = strlen(returnString);
    for (k = 0; k < le && returnString[k] == '0'; ++k);

    if (k == 0)
        return returnString;

    if (le == k)
        k--;

    memmove(returnString, returnString + k, le - k + 1);
    returnString = realloc(returnString, le - k + 1);

    return returnString;
}

char *toDec(struct bignum_fixed b) {
    char *hx = toHex(b.bignum);
    size_t strl = strlen(hx);

    char *result = calloc(sizeof(char), strl * 8);
    char *ret;

    if (b.comma > 0) {
        char *bfComma = calloc(sizeof(char), b.comma + 1);
        memcpy(bfComma, hx, b.comma);
        struct bignum bfC = fromHex(bfComma);
        free(bfComma);
        ret = convertWholeBignumToDecimal(bfC);
        bignumFree(bfC);
    } else {
        ret = "0";
    }

    // concat the result;
    strcat(result, ret);
    result[strlen(ret)] = '.';

    free(ret);

    size_t commaPlaces = strl - b.comma;

    struct bignum afterComma = fromHex(hx + b.comma);
    struct bignum fixedComma = alignDecimalPlaces(afterComma, commaPlaces);

    bignumFree(afterComma);

    alignBignumTo128bit(&fixedComma);
    char *afComma = convertWholeBignumToDecimal(fixedComma);
    strcat(result, afComma);

    bignumFree(fixedComma);
    free(afComma);
    free(hx);

    return result;
}

char *toHexComma(struct bignum_fixed bn) {
    char *hx = toHex(bn.bignum);
    size_t len = strlen(hx);

    if (bn.comma == len)
        return hx;

    bool add = bn.comma == 0;

    char *ret = calloc(sizeof(char),
                       len + 2 + add); // +2 is 1 for , one for \0 and add, if there is an extra zero bf the comma

    if (add)
        ret[0] = '0';
    else
        memcpy(ret, hx, bn.comma);
    ret[bn.comma + add] = '.';

    memcpy(&ret[bn.comma + 1 + add], &hx[bn.comma], len - bn.comma);

    free(hx);

    return ret;
}
