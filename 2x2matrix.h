#pragma  once

#include "bignum.h"

// tl = topleft; br = bottomright etc
struct matrix2x2 {
    struct bignum tl;
    struct bignum tr;
    struct bignum bl;
    struct bignum br;
};

struct matrix2x2 sqrt2Matrix();

struct matrix2x2 unitMatrix();

struct matrix2x2 squareMatrix(struct matrix2x2);

struct matrix2x2 multiplyMatrix(struct matrix2x2 *m1, struct matrix2x2 *m2, bool useThreading);

struct matrix2x2 expMatrix(struct matrix2x2 m, size_t exp);

void freeMatrix(struct matrix2x2 m);

void printMatrix(struct matrix2x2 *m);

struct matrix2x2 expMatrixFast(struct matrix2x2 m, size_t exp);

struct matrix2x2 copyM(struct matrix2x2);