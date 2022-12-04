#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "bignum_fixed.h"

#define DWORD uint32_t
// #define MAX ((DWORD)0xFFFFFFFFFFFFFFFF)
#define MAXi(i, j) (((i) > (j)) ? (i) : (j))

typedef unsigned __int128 uint128_t;

struct bignum {
    uint32_t *data;
    size_t size;
};

void bignumFree(struct bignum b);

struct bignum bignumFromInt64(int64_t);

int64_t int64FromBignum(struct bignum *);

void shiftLeftInPlace(struct bignum *b, uint64_t shl);

void shiftRightInPlace(struct bignum *, uint64_t);

struct bignum add(struct bignum a, struct bignum b);

void add_inplace(struct bignum *a, struct bignum *b, struct bignum *output);

void fastNegation(struct bignum x);

struct bignum slowNegation(struct bignum b);

struct bignum subtraction(struct bignum *a, struct bignum *b);

struct bignum newtondiv(struct bignum *dividend, struct bignum *divisor, size_t s);

uint32_t getint(char *str);

struct bignum multiplication(struct bignum x, struct bignum y);

struct bignum _multiplication(struct bignum x, struct bignum y, bool);

struct bignum fromHex(char *hex);

char *toHex(struct bignum *x);

bool equals(struct bignum a, struct bignum b);

struct bignum copy(struct bignum);

struct bignum goldschmidtDiv(struct bignum *dividend, struct bignum *divisor, size_t k);


struct bignum mul2x128Bit(struct bignum, struct bignum);

void bringToSize(struct bignum *, size_t);

bool isSigned(struct bignum);

uint64_t getBitLength(struct bignum *a);


struct bignum longDivision(struct bignum *dividend, struct bignum *divisor, size_t accuracy);

struct bignum longDivisionWithRemainder(struct bignum *dividend, struct bignum *divisor, struct bignum *remainderOutput,
                                        size_t accuracy);

struct bignum computeFirstGuess(struct bignum *divisor, uint64_t k);

uint64_t computeIterations(uint64_t s);

struct bignum fastExponential(struct bignum x, size_t exponent);
