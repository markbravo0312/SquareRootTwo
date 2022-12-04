#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "bignum.h"
#include "utils.h"
#include <sys/param.h>
#include <unistd.h>
#include "bignum_fixed.h"
#include <math.h>
#include <time.h>
#include <pthread.h>

//TODO don't forget to test if Malloc returned fine!
//TODO FREE IS SO IMPORTANT!


struct bignum bignumFromInt64(int64_t number) {
    size_t size = sizeof(size_t);
    uint32_t *data = malloc(size);
    if (data == NULL) {
        printf("malloc failed\n");
        exit(EXIT_FAILURE);
    }

    memcpy(data, &number, size);

    struct bignum result = {data, 2};
    return result;
}

int64_t int64FromBignum(struct bignum *bignum) {
    int64_t result = 0;
    memcpy(
            &result,
            bignum->data,
            sizeof(int64_t)
    );
    return result;
}

const uint8_t EIGHT_BIT_SHR = 0xFF >> 1;

/*
    Methode die ein bignum mit 0 vergleicht, true wenn gleich 0;
 */
bool isZero(struct bignum *a) {
    bool iszero = true;
    for (size_t i = 0; i < a->size; ++i) {
        if (a->data[i] != 0)
            iszero = false;
    }
    return iszero;
}


void shiftLeftInPlace(struct bignum *b, uint64_t shl) {
    // optimize smaller shifts
    uint8_t *data;
    size_t new_byte_size = b->size * 4;
    if (shl > 8 || b->data[b->size - 1] != 0 || new_byte_size % 8 != 0) {
        new_byte_size = b->size * 4 + (shl >> 3) + 1;

        if (new_byte_size % 8 != 0)
            new_byte_size += 8 - (new_byte_size % 8);

        data = (uint8_t *) realloc(b->data, new_byte_size);
        if (data == NULL) {
            printf("realloc failed\n");
            exit(EXIT_FAILURE);
        }
        assert(new_byte_size % 8 == 0);
    } else {
        data = (uint8_t *) b->data;
    }


    assert(new_byte_size % 8 == 0);

    // first "shift" by just moving the memory
    if (shl >= 8) {
        uint64_t shl_bytes = shl / 8; // 16

        if (new_byte_size == b->size * 4)
            memmove(data + shl_bytes, data, (b->size - 1) * 4);
        else
            memmove(data + shl_bytes, data, b->size * 4);

        // TODO: make faster
        memset(data, 0, shl_bytes);
        if (new_byte_size != b->size * 4)
            memset(data + b->size * 4 + shl_bytes, 0, new_byte_size - b->size * 4 - shl_bytes);

        shl -= shl_bytes * 8;
    } else {
        // todo: change size to uint64_t
        if (b->size * 4 < new_byte_size)
            memset(data + b->size * 4, 0, sizeof(uint64_t));
    }

    b->size = new_byte_size / 4;

    assert (shl < 8);

    bool carry = false;
    while (shl > 0) {
        for (size_t i = 0; i < b->size * 4; ++i) {
            bool had_carry = carry;

            carry = data[i] > EIGHT_BIT_SHR;

            data[i] <<= 1;
            if (had_carry)
                data[i]++;
        }
        shl--;
    }

    b->data = (uint32_t *) data;
}

void shiftRightInPlace(struct bignum *b, uint64_t shr) {
    /*char *hexBefore = toHex(b);
    size_t shrBf = shr;*/

    uint8_t *data = (uint8_t *) b->data;
    size_t new_byte_size = b->size * 4 - (shr >> 3);

    if (new_byte_size % 8 != 0)
        new_byte_size += 8 - (new_byte_size % 8);
    assert(new_byte_size % 8 == 0);


    // first "shift" by just moving the memory
    assert(new_byte_size <= b->size * 4);
    if (shr >= 8) {
        uint64_t eights = shr / 8;

        // allocate new memory
        uint8_t *newData = calloc(new_byte_size, sizeof(uint8_t));
        if (newData == NULL) {
            printf("calloc failed\n");
            exit(EXIT_FAILURE);
        }

        memcpy(newData, data + eights, b->size * 4 - eights);
        shr -= eights * 8;

        free(data);
        data = newData;

        b->size = new_byte_size / 4;
    }

    assert (shr < 8);

    bool carry = false;
    while (shr > 0) {
        for (size_t i = 0; i < b->size * 4; ++i) {

            carry = i < b->size * 4 - 1 && data[i + 1] & 1;

            data[i] >>= 1;

            data[i] += carry << 7;

            carry = false;
        }
        shr--;
    }

    b->data = (uint32_t *) data;

}

size_t rightBitDegree(struct bignum a) {
    size_t deg = 0;
    size_t f = 0;
    for (size_t i = 0; i < a.size; ++i) {
        if (a.data[i] != 0) {
            f = i;
        }
    }

    if (f > 0)
        deg = (f) * 32;

    size_t data = a.data[f];
    while (data != 0) {
        data >>= 1;
        deg++;
    }

    return deg;
}

uint32_t getint(char *str) {
    int length = 0;
    char *beginning = str;
    while ((*str > 47) && (*str < 58)) {
        length++;
        str++;
    }

    uint32_t sum = 0;
    for (; length > 0; length--, beginning++) {
        char ch = *beginning;
        int k = ch - 48;
        sum += (k * (uint32_t) pow(10, length - 1));
    }

    return sum;
}

void copyTo(struct bignum src, struct bignum *dst) {
    if (src.size != dst->size) {
        dst->data = realloc(dst->data, src.size * sizeof(uint32_t));
        if (dst->data == NULL) {
            printf("realloc failed\n");
            exit(EXIT_FAILURE);
        }
        dst->size = src.size;
    }
    memcpy(dst->data, src.data, src.size * sizeof(uint32_t));
}

void add_inplace(struct bignum *a, struct bignum *b, struct bignum *output) {
    if (a->size == 0) {
        if (b->data != NULL && b->data != output->data)
            copyTo(*b, output);
        return;
    }
    if (b->size == 0) {
        if (a->data != NULL && a->data != output->data)
            copyTo(*a, output);
        return;
    }

    // make "a" to be >= "b"
    if (a->size < b->size) {
        struct bignum *swap = b;
        b = a;
        a = swap;
    }

    bool overflow;
    if (output->size > a->size)
        overflow = false;
    else if (a->size > b->size) {
        overflow = a->data[a->size - 1] >> 28 == 0xF;
    } else {
        // size is equal
        overflow =
                (uint64_t) (a->data[a->size - 1]) + (uint64_t) (b->data[b->size - 1]) >
                UINT32_MAX;
    }

    size_t newSize;
    if (overflow) {
        newSize = a->size + (2 - a->size % 2);
    } else {
        newSize = a->size;
    }

    if (output->size < newSize || output->data == NULL) {
        output->data = realloc(output->data, newSize * sizeof(uint32_t));
        if (output->data == NULL) {
            printf("realloc failed\n");
            exit(EXIT_FAILURE);
        }

        if (output->size < newSize)
            memset(output->data + output->size, 0, (newSize - output->size) * sizeof(uint32_t));
    }
    output->size = newSize;

    uint64_t temp;
    uint8_t carry = 0;

    for (size_t j = 0; j < newSize; ++j) {
        temp = carry;
        if (j < a->size)
            temp += (uint64_t) a->data[j];
        if (j < b->size)
            temp += (uint64_t) b->data[j];

        carry = temp >> 32;

        output->data[j] = (uint32_t) temp;
    }
}

struct bignum add(struct bignum a, struct bignum b) {
    if (a.size == 0) {
        struct bignum r = copy(b);
        return r;
    }
    if (b.size == 0) {
        struct bignum r = copy(a);
        return r;
    }

    // make "a" to be >= "b"
    if (a.size < b.size) {
        struct bignum swap = b;
        b = a;
        a = swap;
    }

    bool moreMemory;
    if (a.size > b.size) {
        moreMemory = a.data[a.size - 1] >> 28 == 0xF;
    } else {
        // size is equal
        moreMemory =
                (uint64_t) (a.data[a.size - 1]) + (uint64_t) (b.data[b.size - 1]) >
                UINT32_MAX;
    }


    size_t newSize;
    if (moreMemory) {
        newSize = a.size + (2 - a.size % 2);
    } else {
        newSize = a.size;
    }

    struct bignum res;
    {
        res.data = (uint32_t *) calloc(sizeof(uint32_t), newSize);
        res.size = newSize;
        if (res.data == NULL) {
            printf("calloc failed\n");
            exit(EXIT_FAILURE);
        }
    }


    assert(a.size >= b.size);

    uint64_t temp;
    uint8_t carry = 0;

    for (size_t j = 0; j < newSize; ++j) {
        temp = carry;
        if (j < a.size)
            temp += (uint64_t) a.data[j];
        if (j < b.size)
            temp += (uint64_t) b.data[j];

        carry = temp >> 32;

        res.data[j] = (uint32_t) temp;
    }

    return res;
}

void fastNegation(struct bignum x) {
    for (size_t i = 0; i < x.size; ++i)
        x.data[i] = ~x.data[i];

    bool carry = true;
    for (size_t i = 0; i < x.size; i++) {
        if (carry)
            x.data[i]++;

        carry = x.data[i] == 0;
        if (!carry)
            break;
    }
}

struct bignum slowNegation(struct bignum b) {
    uint32_t *data = (uint32_t *) malloc(sizeof(uint32_t) * b.size);
    if (data == NULL) {
        printf("malloc failed\n");
        exit(EXIT_FAILURE);
    }
    for (size_t k = 0; k < b.size; ++k) {
        data[k] = ~b.data[k];
    }

    struct bignum result = {.data = data, .size = b.size};
    bool carry = true;

    for (size_t i = 0; i < result.size; i++) {
        if (carry)
            result.data[i]++;

        carry = result.data[i] == 0;
        if (!carry)
            break;
    }

    return result;
}


bool isSigned(struct bignum a) {
    return (a.data[a.size - 1] & 0x80000000) != 0;
}

void bringToSize(struct bignum *b, size_t newSize) {
    assert(newSize > b->size);

    size_t m = newSize - b->size;

    b->data = realloc(b->data, newSize * sizeof(uint32_t));
    if (b->data == NULL) {
        printf("realloc failed\n");
        abort();
    }
    memset(&b->data[b->size], 0, m * sizeof(uint32_t));

    b->size = newSize;
}

enum COMPARISON {
    LESS, MORE, EQUAL
};

// compares a to b
// a = 1 b = 2 == MORE
// a = -1 b = 0 == LESS
enum COMPARISON compare(struct bignum a, struct bignum b) {
    bool swapped;
    if (b.size > a.size) {
        struct bignum sw = a;
        a = b;
        b = sw;
        swapped = true;
    } else swapped = false;

    bool signedB = isSigned(b);
    enum COMPARISON result = EQUAL;

    for (size_t i = MAXi(a.size, b.size); i > 0; --i) {
        if (i - 1 < b.size && i - 1 < a.size) {
            if (a.data[i - 1] == b.data[i - 1])
                continue;
            if (a.data[i - 1] < b.data[i - 1]) {
                result = LESS;
                break;
            }
            if (a.data[i - 1] > b.data[i - 1]) {
                result = MORE;
                break;
            }
        }

        if (i - 1 >= b.size) {
            uint32_t cm = signedB ? INT32_MAX : 0;
            if (a.data[i - 1] == cm)
                continue;
            if (a.data[i - 1] < cm) {
                result = LESS;
                break;
            }
            if (a.data[i - 1] > 0) {
                result = MORE;
                break;
            }
        }
    }

    // printf("COMPARE: %zu RESULT eq = %x ls = %x mr = %x size_a = %zu size_b = %zu\n", MAX(a.size, b.size) - i,
    //        result == EQUAL, result == LESS, result == MORE, a.size, b.size);

    if (result == EQUAL)
        return EQUAL;

    if (swapped) {
        if (result == LESS)
            return MORE;
        else
            return LESS;
    }
    return result;
}

void subtraction_inplace(struct bignum *a, struct bignum *_b, struct bignum *out) {
    bool freeSecondParameter;
    struct bignum b;

    if (_b->size < a->size) {
        b = copy(*_b);
        bringToSize(&b, a->size);
        freeSecondParameter = true;
    } else {
        b = *_b;
        freeSecondParameter = false;
    }

    struct bignum neg;
    if (freeSecondParameter) {
        neg = b;
        fastNegation(neg);
    } else {
        neg = slowNegation(b);
    }

    add_inplace(a, &neg, out);

    assert(out->size >= neg.size);
    if (out->size != neg.size) {
        out->size = neg.size;
    }

    if (freeSecondParameter)
        bignumFree(b);
    else
        bignumFree(neg);
}

struct bignum subtraction(struct bignum *a, struct bignum *_b) {
    // printf("Subtraction: 0x%s - 0x%s\n", toHex(a), toHex(_b));

    bool freeSecondParameter;
    struct bignum b;
    if (_b->size < a->size) {
        b = copy(*_b);
        bringToSize(&b, a->size);
        freeSecondParameter = true;
    } else {
        b = *_b;
        freeSecondParameter = false;
    }

    struct bignum neg = slowNegation(b);

    struct bignum subbed = add(*a, neg);


    if (subbed.size != neg.size) {
        subbed.data = realloc(subbed.data, neg.size * sizeof(uint32_t));
        if (subbed.data == NULL) {
            printf("realloc failed\n");
            exit(EXIT_FAILURE);
        }
        subbed.size = neg.size;
    }

    bignumFree(neg);
    if (freeSecondParameter)
        bignumFree(b);


    return subbed;
}

inline struct bignum copy(struct bignum b) {
    uint32_t *dataCpy = calloc(sizeof(uint32_t), b.size);
    if (dataCpy == NULL) {
        printf("calloc failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(dataCpy, b.data, b.size * sizeof(uint32_t));

    struct bignum cp = {dataCpy, b.size};
    return cp;
}

struct bignum mul2x128Bit(struct bignum a, struct bignum b) {
    if (a.size == 0 || b.size == 0)
        return bignumFromInt64(0);

    uint64_t fa = a.data[0];
    if (a.size > 1)
        fa |= (uint64_t) a.data[1] << 32;

    uint64_t fb = b.data[0];
    if (b.size > 1)
        fb |= (uint64_t) b.data[1] << 32;


    uint128_t f = (uint128_t) fa * (uint128_t) fb;

    uint32_t *data = calloc(sizeof(uint128_t), 1);
    if (data == NULL) {
        printf("calloc failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(data, &f, sizeof(uint128_t));
    struct bignum r = {
            data,
            4
    };

    return r;
}

bool qualifiedFor128bitMul(struct bignum a) {
    if (a.size <= 2)
        return true;
    if (a.size == 3)
        return a.data[2] == 0 || a.data[2] == UINT32_MAX;
    if (a.size == 4)
        return (a.data[2] == 0 && a.data[3] == 0) || (a.data[2] == UINT32_MAX && a.data[3] == UINT32_MAX);

    return false;
}

struct bignum_pair getHiLo(struct bignum x, size_t m) {
    struct bignum low = {x.data, MIN(m, x.size)};
    struct bignum hi;
    if (m < x.size) {
        struct bignum t = {x.data + m, x.size - m};
        hi = t;
    } else {
        struct bignum t = {NULL, 0};
        hi = t;
    }

    struct bignum_pair p = {low, hi};
    return p;
}

void *multiplicationThread(void *input) {
    struct bignum_pair *pair = (struct bignum_pair *) input;

    struct bignum *result = malloc(sizeof(struct bignum));
    *result = _multiplication(pair->first, pair->second, false);

    pthread_exit(result);
}

struct bignum _multiplication(struct bignum x, struct bignum y, bool firstLevel) {
    // x0y0 + 2**m((x0+x1)(y0+y1)−x0y0−x1y1) + 2**(2m) x1y1
    if (isZero(&x) || isZero(&y)) {
        struct bignum z = {NULL, 0};
        return z;
    }

    bool qualifiedFor128 = qualifiedFor128bitMul(x) && qualifiedFor128bitMul(y);

    if (qualifiedFor128)
        return mul2x128Bit(x, y);

    size_t max1 = MAXi(x.size, y.size);
    size_t splitting_point = max1 / 2;

    struct bignum_pair xSplit = getHiLo(x, splitting_point);
    struct bignum_pair ySplit = getHiLo(y, splitting_point);

    struct bignum x0 = xSplit.first;
    struct bignum x1 = xSplit.second;

    struct bignum y0 = ySplit.first;
    struct bignum y1 = ySplit.second;

    uint64_t m = splitting_point * 4 * 8;

    struct bignum x0plusx1 = add(x0, x1);
    struct bignum y0plusy1 = add(y0, y1);

    struct bignum x0y0;
    struct bignum x1y1;
    struct bignum x0px1y0py1;
    // recursive step: (x0 * y0)


    // use threading then
    if (firstLevel && (x0.size > 500 || x1.size > 500 || y0.size > 500 || y1.size > 500)) {
        pthread_t t1;
        pthread_t t2;
        pthread_t t3;

        // printf("%zu %zu %zu %zu \n", x0.size, x1.size, y0.size, y1.size);

        struct bignum_pair *pair_1 = malloc(sizeof(struct bignum_pair));
        pair_1->first = x0;
        pair_1->second = y0;

        struct bignum_pair *pair_2 = malloc(sizeof(struct bignum_pair));
        pair_2->first = x1;
        pair_2->second = y1;

        struct bignum_pair *pair_3 = malloc(sizeof(struct bignum_pair));
        pair_3->first = x0plusx1;
        pair_3->second = y0plusy1;

        pthread_create(&t1, NULL, multiplicationThread, pair_1);
        pthread_create(&t2, NULL, multiplicationThread, pair_2);
        pthread_create(&t3, NULL, multiplicationThread, pair_3);

        void *result1;
        void *result2;
        void *result3;

        pthread_join(t1, &result1);
        pthread_join(t2, &result2);
        pthread_join(t3, &result3);

        x0y0 = *((struct bignum *) result1);
        x1y1 = *((struct bignum *) result2);
        x0px1y0py1 = *((struct bignum *) result3);

        free(pair_1);
        free(pair_2);
        free(pair_3);

        free(result1);
        free(result2);
        free(result3);
    } else {
        x0y0 = _multiplication(x0, y0, false);
        x1y1 = _multiplication(y1, x1, false);
        x0px1y0py1 = _multiplication(x0plusx1, y0plusy1, false);
    }


    // recursive step: (x1 * y1)

    subtraction_inplace(&x0px1y0py1, &x0y0, &x0px1y0py1);
    subtraction_inplace(&x0px1y0py1, &x1y1, &x0px1y0py1);

    shiftLeftInPlace(&x1y1, 2 * m);
    shiftLeftInPlace(&x0px1y0py1, m);

    add_inplace(&x0y0, &x0px1y0py1, &x0y0);
    add_inplace(&x0y0, &x1y1, &x0y0);

    bignumFree(x1y1);
    bignumFree(x0plusx1);
    bignumFree(y0plusy1);
    bignumFree(x0px1y0py1);

    return x0y0;
}


struct bignum multiplication(struct bignum x, struct bignum y) {
    return _multiplication(x, y, true);
}

void bignumFree(struct bignum b) {
    free(b.data);
    b.data = NULL;
    b.size = 0;
}

uint8_t translateHex(char *str) {
    return strtol(str, NULL, 16);
}

struct bignum fromHex(char *hex) {
    size_t length = strlen(hex);

    bool possibleOOB = false;
    if (length % 2 == 1) {
        length++;
        possibleOOB = true;
    }

    size_t numLength = length / 2;
    if (numLength % sizeof(uint128_t) != 0)
        numLength += sizeof(uint128_t) - (numLength % sizeof(uint128_t));

    assert(numLength % sizeof(uint64_t) == 0);
    uint8_t *fromHexData = calloc(sizeof(uint8_t), numLength);
    if (fromHexData == NULL) {
        printf("calloc failed\n");
        exit(EXIT_FAILURE);
    }

    if (hex[length - 1] == 0 && hex[length - 2] != 0)
        length--;

    uint8_t hexNo;
    for (size_t i = 0; i < length; i += 2) {
        // Todo: investigate linter error
        char plusOne = possibleOOB && length - i <= 1 ? '0' : hex[length - i - 2];

        char charPair[3] = {plusOne, hex[length - i - 1], 0};
        hexNo = translateHex(charPair);
        fromHexData[i / 2] = hexNo;
    }

    struct bignum b = {(uint32_t *) fromHexData, numLength / 4};
    return b;
}

bool equals(struct bignum a, struct bignum b) {
    if (b.size > a.size) {
        struct bignum sw = b;
        b = a;
        a = sw;
    }

    assert(a.size >= b.size);

    bool ret = true;
    for (size_t i = 0; i < a.size; ++i) {
        if (i >= b.size) {
            if (a.data[i] != 0) {
                printf("#eq: (false_of) i=%zu a(i)= %x b(i) = %x\n", i, a.data[i], 0);
                ret = false;
            } else {
                continue;
            }
        }

        if (a.data[i] != b.data[i]) {
            printf("#eq: (false) i=%zu a(i)= %x b(i) = %x\n", i, a.data[i], b.data[i]);
            ret = false;
        } else {
            printf("#eq: (true) i=%zu a(i)= %x b(i) = %x\n", i, a.data[i], b.data[i]);
        }
    }
    return ret;
}

char *toHex(struct bignum *x) {
    // printf("\n\nToHex:\n\n");
    size_t sz = x->size * 8;

    char *res = calloc(sizeof(uint8_t), sz + 1);
    if (res == NULL) {
        printf("calloc failed\n");
        exit(EXIT_FAILURE);
    }
    char lookup[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A',
                     'B', 'C', 'D', 'E', 'F'};

    uint8_t *cas = (uint8_t *) x->data;
    for (size_t j = 0; j < x->size * 4; ++j) {
        uint8_t c = cas[j];
        uint8_t left = c >> 4;
        uint8_t right = c & 0xF;

        res[sz - j * 2 - 2] = lookup[left];
        //if (j == 0 && right == 0)
        //    continue;

        res[sz - j * 2 - 1] = lookup[right];
    }

    size_t i = 0;
    for (; i < sz; ++i) {
        if (res[i] != '0')
            break;
    }

    memmove(res, res + i, sz - i + 1);
    res = realloc(res, sz - i + 2);
    if (res == NULL) {
        printf("realloc failed\n");
        exit(EXIT_FAILURE);
    }
    res[sz - i + 1] = 0;

    return res;
}

uint64_t getBitLength(struct bignum *a) {
    int64_t mostSignificantIndex;
    uint64_t n, t, k;
    mostSignificantIndex = (a->size) - 1;
    n = a->data[mostSignificantIndex];

    if (mostSignificantIndex) {
        for (; mostSignificantIndex >= 0; mostSignificantIndex--) {
            if (a->data[mostSignificantIndex] != 0) {
                n = a->data[mostSignificantIndex];
                break;
            }
        }
    }
    k = 1;
    t = 1;
    while (t < n) {
        t *= 2;
        k++;
    }

    return k + (mostSignificantIndex * 32);
}

struct bignum ldHelperIterative(struct bignum *dividend, struct bignum *divisor, struct bignum *remainder_output,
                                size_t decimalPlaces) {
    assert(!isZero(divisor));

    struct bignum one = bignumFromInt64(1);

    struct bignum dividendCopy = copy(*dividend);
    struct bignum quotient = bignumFromInt64(0);
    struct bignum remainder = bignumFromInt64(0);

    struct bignum tempQuotient = bignumFromInt64(0);
    struct bignum tempQuotientInner = bignumFromInt64(1);


    for (size_t i = 0; i < decimalPlaces / 2 + 1; ++i) {
        if (i == 1) {
            bignumFree(dividendCopy);
            dividendCopy = remainder;
        }

        if (i != 0) {
            if (isZero(&dividendCopy))
                break;

            shiftLeftInPlace(&quotient, 8);

            shiftLeftInPlace(&dividendCopy, 8);

            bignumFree(tempQuotient);
            tempQuotient = bignumFromInt64(0);
        }

        while (true) {
            enum COMPARISON cmp = compare(dividendCopy, *divisor);
            if (cmp == EQUAL) {
                add_inplace(&tempQuotient, &one, &tempQuotient);
                memset(remainder.data, 0, remainder.size * sizeof(uint32_t));
                break;
            } else if (cmp == LESS) {
                if (i == 0) {
                    bignumFree(remainder);
                    remainder = copy(dividendCopy);
                }
                break;
            }

            struct bignum divisor_cp = copy(*divisor);

            bignumFree(tempQuotientInner);
            tempQuotientInner = copy(one);

            while (compare(divisor_cp, dividendCopy) != MORE) {
                shiftLeftInPlace(&divisor_cp, 1);
                shiftLeftInPlace(&tempQuotientInner, 1);
            }

            if (compare(dividendCopy, divisor_cp) == LESS) {
                shiftRightInPlace(&divisor_cp, 1);
                shiftRightInPlace(&tempQuotientInner, 1);
            }

            // subtract the divisor from the dividend -> long division subtraction
            subtraction_inplace(&dividendCopy, &divisor_cp, &dividendCopy);
            add_inplace(&tempQuotient, &tempQuotientInner, &tempQuotient);

            bignumFree(divisor_cp);
        }

        add_inplace(&quotient, &tempQuotient, &quotient);
    }


    if (remainder_output != NULL)
        *remainder_output = remainder;

    bignumFree(tempQuotient);
    bignumFree(tempQuotientInner);
    bignumFree(dividendCopy);
    bignumFree(one);

    return quotient;
}

struct bignum longDivision(struct bignum *dividend, struct bignum *divisor, size_t accuracy) {
    struct bignum result2 = ldHelperIterative(dividend, divisor, NULL, accuracy);

    // TODO: set the comma correctly somehow

    return result2;
}

struct bignum longDivisionWithRemainder(struct bignum *dividend, struct bignum *divisor, struct bignum *remainderOutput,
                                        size_t accuracy) {
    return ldHelperIterative(dividend, divisor, remainderOutput, accuracy);
}

//k = Shift factor for constants => Number of decimal places
//divisor should be scaled between 0.5 and 1
//For Optimal first guess, 48/17 - 32/17 *
struct bignum computeFirstGuess(struct bignum *divisor, uint64_t k) {
    // a =  48/17 * 2*32 = 2.83 
    struct bignum a = bignumFromInt64(12126966484);

    // b = 32/17 * 2^32 = 1.89 
    struct bignum b = bignumFromInt64(8084644322);

    //Mulitply b with Divisor scaled: 0.5 < Divisor < 1
    uint32_t shiftBits = k - 32;


    if (shiftBits) {
        shiftLeftInPlace(&a, shiftBits);
        shiftLeftInPlace(&b, shiftBits);
    }


    struct bignum r = multiplication(b, *divisor);
    shiftRightInPlace(&r, k);

    subtraction_inplace(&a, &r, &a);

    bignumFree(b);
    bignumFree(r);


    return a;

}


uint64_t computeIterations(uint64_t s) {
    double sevent = (double) log2(17);

    uint64_t result = log2((double) ((s + 1) / sevent));
    return result + 1;
}

struct bignum newtondiv(struct bignum *_dividend, struct bignum *_divisor, size_t s) {
    struct bignum dividend = copy(*_dividend);
    struct bignum divisor = copy(*_divisor);

    if (isZero(&divisor)) {
        fprintf(stderr, "Arithmetic Exception: DivByZero");
        exit(EXIT_FAILURE);
    }


    if (isZero(&dividend)) {
        struct bignum e = {NULL, 0};
        return e;
    }


    // from here on we assume both dividend and divisor to be positive
    assert(!isSigned(dividend));
    assert(!isSigned(divisor));


    uint64_t decimalplaces;
    uint64_t bitsdiv = getBitLength(&divisor);



    // printf("bitsdiv = %d\n", bitsdiv);
    uint64_t shiftFactor;


    // //FOR PRECISE RESULT
    if (s < 16) {
        decimalplaces = bitsdiv * 2;
    } else {
        uint64_t l = (s / bitsdiv) + 1;
        decimalplaces = bitsdiv * l;
    }




    //SCALE DIVISIOR BETWEEN 0 < Divisor < 1, apply same scale to DIVIDEND
    if (bitsdiv < decimalplaces) {
        shiftFactor = decimalplaces - bitsdiv;
        shiftLeftInPlace(&divisor, shiftFactor);
        shiftLeftInPlace(&dividend, shiftFactor);
    }


    struct bignum x = computeFirstGuess(&divisor, decimalplaces);


    struct bignum two = bignumFromInt64(2);
    shiftLeftInPlace(&two, decimalplaces);

    uint64_t t = computeIterations(s);


    if (t < 20) {
        t = 20;
    }

    for (uint64_t i = 0; i < t; i++) {


        struct bignum f = multiplication(divisor, x);
        shiftRightInPlace(&f, decimalplaces);


        subtraction_inplace(&two, &f, &f);


        struct bignum w = multiplication(x, f);
        shiftRightInPlace(&w, decimalplaces);


        bignumFree(x);
        x = w;

        bignumFree(f);
    }


    struct bignum quot = multiplication(x, dividend);
    shiftRightInPlace(&quot, decimalplaces);


    bignumFree(x);
    bignumFree(two);

    if (decimalplaces > s) {
        uint64_t r = decimalplaces - s;
        shiftRightInPlace(&quot, r);
    }

    bignumFree(divisor);
    bignumFree(dividend);

    return quot;

}

//k - Number of binary decimal places result should have, 
//
struct bignum goldschmidtDiv(struct bignum *_dividend, struct bignum *_divisor, size_t k) {
    struct bignum dividend = copy(*_dividend);
    struct bignum divisor = copy(*_divisor);

    uint64_t decimalplaces;
    uint64_t bitsdiv = getBitLength(&divisor);
    uint64_t shiftFactor;

    // CALCULATE WITH DOUBLE PRECISION OF K, IF K < 16, PRECISION = 32 BITS

    if (k < 16) {
        decimalplaces = bitsdiv;
    } else {
        //MUSS SO SEIN UM GENAUES ERGEBNIS zu LIEFERN
        uint64_t l = (k / bitsdiv) + 1;
        decimalplaces = bitsdiv * l;
    }



    //SCALE DIVISIOR BETWEEN 0 < Divisor < 1, apply same scale to DIVIDEND
    if (bitsdiv < decimalplaces) {
        shiftFactor = decimalplaces - bitsdiv;
        shiftLeftInPlace(&divisor, shiftFactor);
        shiftLeftInPlace(&dividend, shiftFactor);
    } else if (bitsdiv > decimalplaces) {
        shiftFactor = bitsdiv - decimalplaces;
        shiftRightInPlace(&divisor, shiftFactor);
        shiftRightInPlace(&dividend, shiftFactor);
    } else {
        shiftFactor = 0;
    }

    //printf("Shiftfactor is equal to: %lu\n", shiftFactor);


    struct bignum Factor;
    struct bignum one = bignumFromInt64(1);
    shiftLeftInPlace(&one, decimalplaces);
    struct bignum two = bignumFromInt64(2);
    shiftLeftInPlace(&two, decimalplaces);
    struct bignum error = bignumFromInt64(1);


    if (bitsdiv > 64) {
        int o = bitsdiv - 64;
        shiftLeftInPlace(&error, o);
    }


    //while(compare(deviation, error) == MORE) {
    for (int i = 0; i < 8; i++) {

        Factor = subtraction(&two, &divisor);


        divisor = multiplication(divisor, Factor);
        shiftRightInPlace(&divisor, decimalplaces);


        dividend = multiplication(dividend, Factor);
        shiftRightInPlace(&dividend, decimalplaces);


        //if divisor is larger or equal to one break
        if (compare(divisor, one) == MORE || compare(divisor, one) == EQUAL) {
            break;
        }


        bignumFree(Factor);
    }


    bignumFree(two);
    bignumFree(one);
    bignumFree(error);


    uint64_t correction;
    //COMMENT BELOW FOR MAX PRECISION RESULT
    if (k < decimalplaces) {
        correction = decimalplaces - k;
        shiftRightInPlace(&dividend, correction);
    } else if (k > decimalplaces) {
        correction = k - decimalplaces;
        shiftLeftInPlace(&dividend, correction);
    } else {
        correction = 0;
    }

    // shiftRightInPlace(&dividend, correction);


    bignumFree(divisor);
    bignumFree(dividend);


    return dividend;
}

struct bignum fastExponential(struct bignum x, size_t exponent) {
    if (exponent == 0)
        return bignumFromInt64(1);
    if (exponent == 1)
        return copy(x);
    if (exponent == 2)
        return multiplication(x, x);

    size_t num;
    size_t n = 0;
    while (true) {
        if (powl(2, n) > exponent) {
            num = n;
            break;
        }
        n++;
    }

    struct bignum *storage = malloc(sizeof(struct bignum) * num);
    if (storage == NULL) {
        printf("malloc failed\n");
        abort();
    }
    storage[0] = x;

    num--;

    for (size_t i = 1; i <= num; ++i) {
        struct bignum squared = multiplication(storage[i - 1], storage[i - 1]);
        storage[i] = squared;
    }

    struct bignum result = bignumFromInt64(1);

    for (size_t i = 0; i <= num; i++) {
        if ((exponent & 1) == 1) {
            struct bignum r = multiplication(result, storage[i]);
            bignumFree(result);
            result = r;
        }
        exponent >>= 1;
    }

    for (size_t i = 1; i <= num; ++i) {
        bignumFree(storage[i]);
    }
    free(storage);

    return result;
}

