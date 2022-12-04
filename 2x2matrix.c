#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <pthread.h>
#include "utils.h"
#include "2x2matrix.h"

struct matrix2x2 squareMatrix(struct matrix2x2 m) {
    // this is a function specific to the {{0,1},{1,2}} Matrix.
    // it doesn't work otherwise

    // first square all single components
    // bl can be ignored, because it always has the same value as tr
    struct bignum tlSq = multiplication(m.tl, m.tl);
    struct bignum trSq = multiplication(m.tr, m.tr);
    struct bignum brSq = multiplication(m.br, m.br);

    // now tr * tl and tr * br
    struct bignum trTl = multiplication(m.tr, m.tl);
    struct bignum trBr = multiplication(m.tr, m.br);

    // now fuse everything together
    struct matrix2x2 result = {};

    struct bignum tl = add(tlSq, trSq);
    result.tl = tl;

    struct bignum tr = add(trTl, trBr);
    result.tr = tr;
    result.bl = tr;

    struct bignum br = add(trSq, brSq);
    result.br = br;

    // prevent memory leaks
    bignumFree(tlSq);
    bignumFree(trSq);
    bignumFree(brSq);
    bignumFree(trTl);
    bignumFree(trBr);

    return result;
}


struct matrix2x2 sqrt2Matrix() {
    struct bignum tl = bignumFromInt64(0);
    struct bignum tr = bignumFromInt64(1);
    struct bignum br = bignumFromInt64(2);

    struct matrix2x2 result = {
            tl, tr, tr, br
    };
    return result;
}


struct matrix2x2 unitMatrix() {
    struct bignum tl = bignumFromInt64(1);
    struct bignum tr = bignumFromInt64(0);
    struct bignum bl = bignumFromInt64(0);
    struct bignum br = bignumFromInt64(1);

    struct matrix2x2 result = {tl, tr, bl, br};

    return result;
}

void *threadingHelper(void *inp) {
    struct bignum **p2;
    p2 = ((struct bignum **) inp);
    struct bignum tlA = multiplication(*p2[0], *p2[1]);
    struct bignum tlB = multiplication(*p2[2], *p2[3]);

    struct bignum tl = add(tlA, tlB);

    struct bignum *ptr = malloc(sizeof(struct bignum));
    if(ptr == NULL){
        printf("malloc failed\n");
        exit(EXIT_FAILURE);
    }
    *ptr = tl;

    bignumFree(tlA);
    bignumFree(tlB);

    pthread_exit(ptr);
}

struct matrix2x2 multiplyMatrixThreaded(struct matrix2x2 m1, struct matrix2x2 m2) {
    pthread_t tA;
    pthread_t tB;
    pthread_t tC;
    pthread_t tD;

    struct bignum **pcA = malloc(sizeof(struct bignum *) * 4 * 4);
    struct bignum **pcB = pcA + 4;
    struct bignum **pcC = pcA + 8;
    struct bignum **pcD = pcA + 12;

    assert(pcA != NULL);

    pcA[0] = &m1.tl;
    pcA[1] = &m2.tl;
    pcA[2] = &m1.tr;
    pcA[3] = &m2.bl;

    pcB[0] = &m1.tl;
    pcB[1] = &m2.tr;
    pcB[2] = &m1.tr;
    pcB[3] = &m2.br;

    pcC[0] = &m1.bl;
    pcC[1] = &m2.tl;
    pcC[2] = &m1.br;
    pcC[3] = &m2.bl;

    pcD[0] = &m1.bl;
    pcD[1] = &m2.tr;
    pcD[2] = &m1.br;
    pcD[3] = &m2.br;

    pthread_create(&tA, NULL, threadingHelper, pcA);
    pthread_create(&tB, NULL, threadingHelper, pcB);
    pthread_create(&tC, NULL, threadingHelper, pcC);
    pthread_create(&tD, NULL, threadingHelper, pcD);

    void *_tl;
    pthread_join(tA, &_tl);

    void *_tr;
    pthread_join(tB, &_tr);


    void *_bl;
    pthread_join(tC, &_bl);

    void *_br;
    pthread_join(tD, &_br);


    struct bignum tl = *((struct bignum *) _tl);
    struct bignum tr = *((struct bignum *) _tr);
    struct bignum bl = *((struct bignum *) _bl);
    struct bignum br = *((struct bignum *) _br);

    free(pcA);
    free(_tl);
    free(_tr);
    free(_bl);
    free(_br);

    struct matrix2x2 result = {tl, tr, bl, br};
    return result;
}

const size_t thresh = 3500;

struct matrix2x2 multiplyMatrix(struct matrix2x2 *m1, struct matrix2x2 *m2, bool useThreading) {
    if (useThreading) {
        if (m1->br.size > thresh || m2->br.size > thresh)
            return multiplyMatrixThreaded(*m1, *m2);
    }

    // maybe make a specified version of this for our matrix
    // this would save 1 mul in the end

    // this means 2 multiplications per cell now == 8 muls

    // tl:

    struct bignum tlA = multiplication(m1->tl, m2->tl);
    struct bignum tlB = multiplication(m1->tr, m2->bl);

    struct bignum tl = add(tlA, tlB);

    bignumFree(tlA);
    bignumFree(tlB);

    // tr:
    struct bignum trA = multiplication(m1->tl, m2->tr);
    struct bignum trB = multiplication(m1->tr, m2->br);

    struct bignum tr = add(trA, trB);

    bignumFree(trA);
    bignumFree(trB);


    // bl:
    struct bignum blA = multiplication(m1->bl, m2->tl);
    struct bignum blB = multiplication(m1->br, m2->bl);

    struct bignum bl = add(blA, blB);

    bignumFree(blA);
    bignumFree(blB);

    // br:
    struct bignum brA = multiplication(m1->bl, m2->tr);
    struct bignum brB = multiplication(m1->br, m2->br);

    struct bignum br = add(brA, brB);

    bignumFree(brA);
    bignumFree(brB);

    struct matrix2x2 result = {tl, tr, bl, br};


    return result;
}


inline void printMatrix(struct matrix2x2 *m) {
    char *tl = toHex(&m->tl);
    char *tr = toHex(&m->tr);
    char *bl = toHex(&m->bl);
    char *br = toHex(&m->br);

    printf("Matrix: \n");
    printf("TL: 0x%s\n", tl);
    printf("TR: 0x%s\n", tr);
    printf("BL: 0x%s\n", bl);
    printf("BR: 0x%s\n", br);
    printf("\n");

    free(tl);
    free(tr);
    free(bl);
    free(br);
}

struct matrix2x2 copyM(struct matrix2x2 m) {
    struct bignum tl = copy(m.tl);
    struct bignum tr = copy(m.tr);
    struct bignum br = copy(m.br);

    struct matrix2x2 m2 = {};
    m2.tl = tl;
    m2.tr = tr;
    m2.br = br;
    if (m.bl.data != m.tr.data) {
        struct bignum bl = copy(m.bl);
        m2.bl = bl;
    } else
        m2.bl = tr;

    return m2;
}

struct matrix2x2 expMatrix(struct matrix2x2 m, size_t exp) {
    struct matrix2x2 mcp = copyM(m);
    struct matrix2x2 res = unitMatrix();

    while (exp > 0) {
        if (exp % 2 == 1) {
            struct matrix2x2 n = multiplyMatrix(&res, &mcp, true);
            freeMatrix(res);
            res = n;
        }
        struct matrix2x2 n = squareMatrix(mcp);
        freeMatrix(mcp);
        mcp = n;
        exp /= 2;
    }

    freeMatrix(mcp);

    return res;
}

struct matrix2x2 expMatrixFast(struct matrix2x2 m, size_t exp) {
    size_t exp_no;
    size_t ind = 0;

    while (true) {
        if (powl(2, ind) > exp) {
            exp_no = ind;
            break;
        }
        ind++;
    }

    struct matrix2x2 *storage = malloc(sizeof(struct matrix2x2) * exp_no);
    if(storage == NULL){
        printf("malloc failed\n");
        exit(EXIT_FAILURE);
    }
    storage[0] = m;

    exp_no--;


    for (size_t i = 1; i <= exp_no; ++i) {
        struct matrix2x2 squared = squareMatrix(storage[i - 1]);

        storage[i] = squared;
    }

    struct matrix2x2 result = unitMatrix();

    for (size_t i = 0; i <= exp_no; i++) {
        if ((exp & 1) == 1) {
            struct matrix2x2 r = multiplyMatrix(&result, &storage[i], true);
            freeMatrix(result);
            result = r;
        }
        exp >>= 1;
    }

    for (size_t i = 1; i <= exp_no; ++i) {
        freeMatrix(storage[i]);
    }
    free(storage);

    return result;
}

void freeMatrix(struct matrix2x2 m) {
    bignumFree(m.tr);
    bignumFree(m.tl);
    bignumFree(m.br);

    if (m.bl.data != m.tr.data)
        bignumFree(m.bl);
}