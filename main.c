#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "benchmarking.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/param.h>
#include <ctype.h>
#include "bignum.h"
#include "2x2matrix.h"
#include <math.h>

#include "tests.h"
#include "bignum_fixed.h"


// V0 = fastExp & newtonRaphson division
// V1 = fastExp & longDivision
// V2 = slowExp & newtonRaphson division
// V3 = slowExp & longDivision
size_t implementation = 0;
bool debug = false;

// n = accuracy
// s = fixed point decimal places
struct bignum sqrt2(uint64_t n, size_t s) {
    struct matrix2x2 base = sqrt2Matrix();

    struct matrix2x2 exp;
    if (implementation == 2 || implementation == 3)
        exp = expMatrix(base, n);
    else if (implementation == 0 || implementation == 1)
        exp = expMatrixFast(base, n);
    else {
        fprintf(stderr, "Invalid Implementation version %zu\n", implementation);
        exit(EXIT_FAILURE);
    }

    if (debug) {
        printf("X = %s\n", toHex(&exp.tr));
        printf("X + 1 = %s\n", toHex(&exp.br));
    }

    // for each division calculate 32 places more, to ensure 100% accuracy
    struct bignum div;
    if (implementation == 0 || implementation == 2)
        div = newtondiv(&exp.tr, &exp.br, s + 32 + (4 - s % 4));
    else if (implementation == 1 || implementation == 3)
        div = longDivision(&exp.tr, &exp.br, s / 4 + 32);
    else {
        fprintf(stderr, "Invalid Implementation version %zu\n", implementation);
        exit(EXIT_FAILURE);
    }

    if (debug)
        printf("Div Value bf add = %s\n", toHex(&div));

    struct bignum one = bignumFromInt64(1);

    // first calculate the real shift value
    size_t shf_i;
    for (shf_i = div.size - 1; shf_i > 0 && div.data[shf_i] == 0; shf_i--);

    uint32_t v = div.data[shf_i];
    size_t shf = shf_i * 32;

    do {
        shf += 4;
    } while ((v = v >> 4) != 0);


    // simulate an addition with comma
    shiftLeftInPlace(&one, shf);
    add_inplace(&div, &one, &div);

    if (debug)
        printf("Div Value af add = %s\n", toHex(&div));


    freeMatrix(exp);
    freeMatrix(base);

    bignumFree(one);

    return div;
}

void printHelp() {
    printf("-V<int> :     Die Implementierung, welche verwendet werden soll.\n"
           "              Wenn diese Option nicht gesetzt wird, wird die Hauptimplementierung (-v0) ausgeführt.\n"
           "              Gültige Implementierungsversionen sind v0-3.\n"
           "              Fast-Exponentiation = F; Naive-Exponentiation = E; Newton-Division = N; Long-Division = L;\n"
           "              V0 = F&N; V1 = F&L; V2 = E&N; V3 = E&L\n"
           "                    z.B: ./Implementierung -V1      für Version 1\n\n");

    printf("-B<int> :     Falls gesetzt, wird die Laufzeit der angegebenen Implementierung gemessen und ausgegeben.\n"
           "              Das optionale Argument dieser Option gibt die Anzahl an Wiederholungen des Funktionsaufrufs an. Standardwert ist 10 Wiederholungen.\n"
           "                    z.B: ./Implementierung -B56     für 56 Wiederholungen.\n\n");

    printf("-d<int> :     Ausgabe von n dezimalen Nachkommastellen. Standardwert ist 6.\n"
           "                    z.B: ./Implementierung -d15     für 15 Nachkommastellen. \n\n");

    printf("-h<int> :     Ausgabe von n hexadezimalen Nachkommastellen. Standardwert ist 6\n "
           "                    z.B: ./Implementierung -h15     für 15 Nachkommastellen.\n\n");

    printf("-h / --help : Eine Beschreibung aller Optionen des Programms und Verwendungsbeispiele werden ausgegeben und das Programm danach beendet.\n"
           "                    z.B: ./Implementierung -h    oder   ./Implementierung --help \n\n");
}


int main(int argc, char *argv[]) {
    /* Version Of Implementation */

    /* Number of iterations per benchmark, DEFAULT = 10 */
    bool runBenchmarks = false;
    bool benchImplementation = false;
    bool runTests = false;
    size_t repetitions = 10;

    /*  Booleans to specify answer TYPE: HEX or Decimal  */
    bool inDecimal = false;
    bool inHex = false;

    /* Number of Decimal Places / NachkommaStellen for Hex and Decimal, DEFAULT = 6 */
    size_t decimalPlacesDec = 6;
    size_t decimalPlacesHex = 6;

    char *current;  // pointer to help with Iterating over each char in argv[i]
    for (int i = 1; i < argc; ++i) {
        current = argv[i];
        // handle help
        // check if first character is a -
        if (strlen(current) < 2 || current[0] != '-') {
            fprintf(stderr, "Wrong use of the program. Please use -h or --help for help.\n");
            exit(1);
        }
        current++;

        switch (tolower(*current)) {
            case 't':
                runTests = true;
                break;
            case 'v':
                current++;
                if (!isdigit(*current)) {
                    fprintf(stderr, "You have to enter an implementation version number between 0 and 3 (-v)\n");
                    return EXIT_FAILURE;
                }
                implementation = getint(current);

                if (implementation > 3) {
                    fprintf(stderr, "You have to enter an implementation version number between 0 and 3 (-v)\n");
                    return EXIT_FAILURE;
                }

                break;
            case 'k':
                debug = true;
                break;
            case 'r':
                current++;
                runBenchmarks = true;

                if (*current == '\0') {
                    break;
                }
                if (!isdigit(*current)) {
                    fprintf(stderr, "You have to enter a positive number of benchmark repetitions (-r)\n");
                    printHelp();
                    return EXIT_FAILURE;
                }
                repetitions = getint(current);
                break;
            case 'b':
                current++;
                benchImplementation = true;
                if (*current == '\0')
                    break;
                if (!isdigit(*current)) {
                    fprintf(stderr, "You have to enter a positive number of benchmark repetitions (-b<int>)\n");
                    printHelp();

                    return EXIT_FAILURE;
                }
                repetitions = getint(current);

                if (repetitions == 0) {
                    fprintf(stderr, "The number of repetitions has to be greater than zero.\n");
                    printHelp();

                    return EXIT_FAILURE;
                }
                break;

            case 'd':
                current++;
                inDecimal = true;

                if (*current == '\0')
                    break;
                if (!isdigit(*current)) {
                    fprintf(stderr, "You have to enter a positive Number for Decimal places (-d)\n");
                    printHelp();
                    return EXIT_FAILURE;
                }
                decimalPlacesDec = getint(current);
                break;
            case 'h':
                current++;
                inHex = true;
                if (*current < 58 && *current > 47) {           // -h<int> = hexadecimal
                    decimalPlacesHex = getint(current);
                    break;
                } else {
                    if (*current == 0) {
                        printHelp();
                        return EXIT_SUCCESS;
                    }
                    fprintf(stderr, "You have to enter a positive Number for Hexadecimal places (-h)\n");
                    printHelp();
                    return EXIT_FAILURE;
                }
            case '-':
                current++;
                if (strcmp(current, "help") == 0) {
                    printHelp();
                    return EXIT_SUCCESS;
                } else {
                    fprintf(stdout, "Invalid Parameter --%s\n", current);
                    return EXIT_FAILURE;
                }
        }
    }

    if (runTests) {
        main_test();
        return EXIT_SUCCESS;
    }

    if (runBenchmarks) {
        benchMultiplication(repetitions);
        benchDivision(repetitions);
        benchMatrixExp(repetitions);
        return EXIT_SUCCESS;
    }

    if (!(inHex || inDecimal)) {
        inHex = true;
        inDecimal = true;
    }

    printf("Algorithms used: Exponentiation: ");
    if (implementation == 0 || implementation == 1) {
        printf("Fast Exponentiation");
    } else {
        printf("Naive Exponentiation");
    }
    printf(" Division: ");
    if (implementation == 0 || implementation == 2) {
        printf("Newton-Raphson Division");
    } else {
        printf("Long-Division");
    }
    printf("\n");

    size_t binaryPlacesHex = decimalPlacesHex * 4;
    size_t binaryPlacesDec = ceill((long double) decimalPlacesDec * 3.33);

    // calculate accuracy
    size_t binaryPlaces = MAX(inHex ? binaryPlacesHex : 0, inDecimal ? binaryPlacesDec : 0);
    size_t accuracy = MAX(binaryPlaces, 250);

    // stop division by zero
    repetitions = MAX(1, repetitions);
    if (!benchImplementation)
        repetitions = 1;

    long double sumAll = 0;
    long double sumCalculation = 0;
    long double sumOutput = 0;
    for (size_t i = 0; i < repetitions; ++i) {
        long double startAll = curtime();
        long double startCalc = curtime();
        if (binaryPlaces == 0) {
            if (inDecimal) {
                printf("SQRT ( 2 ) base 10 = 1\n");
                return EXIT_SUCCESS;
            } else {
                printf("SQRT ( 2 ) base 16 = 1\n");
                return EXIT_SUCCESS;
            }
        }


        struct bignum sq2 = sqrt2(accuracy, binaryPlaces);
        struct bignum_fixed fixedSq2 = {&sq2, 1};

        long double timeCalc = curtime() - startCalc;

        long double startOut = curtime();
        if (inDecimal) {
            char *decimalValue = toDec(fixedSq2);
            size_t dcLen = strlen(decimalValue);

            if (dcLen - 2 > decimalPlacesDec) {
                decimalValue = realloc(decimalValue, decimalPlacesDec + 3);
                decimalValue[decimalPlacesDec + 2] = '\0';
            }

            printf("SQRT ( 2 ) base 10 = %s\n", decimalValue);
            free(decimalValue);
        }

        if (inHex) {
            char *decimalValue = toHexComma(fixedSq2);
            size_t dcLen = strlen(decimalValue);

            if (dcLen - 2 > decimalPlacesHex) {
                decimalValue = realloc(decimalValue, decimalPlacesHex + 3);
                decimalValue[decimalPlacesHex + 2] = '\0';
            }


            printf("SQRT ( 2 ) base 16 = %s\n", decimalValue);
            free(decimalValue);
        }
        long double timeOut = curtime() - startOut;
        long double timeAll = curtime() - startAll;

        if (benchImplementation)
            printf("Benchmark: Iteration (%zu), t(calculation) = %.6Lfs; t(output) = %.6Lfs; t(complete) = %.6Lfs;\n",
                   i + 1, timeCalc, timeOut, timeAll);

        sumCalculation += timeCalc;
        sumOutput += timeOut;
        sumAll += timeAll;

        bignumFree(sq2);
    }

    if (benchImplementation)
        printf("\nBenchmark Results: no. of iterations: %zu, avg(calculation) = %.6Lfs; avg(output) = %.6Lfs; avg(complete) = %.6Lfs;\n",
               repetitions, sumCalculation / repetitions, sumOutput / repetitions, sumAll / repetitions);

    return EXIT_SUCCESS;
}







