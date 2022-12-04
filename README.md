# SquareRootTwo

This Program calculates n digits of the square root of Two to O(log(n)^2).

A fixed point struct **bignum** is used with optimized arithmetic. *Karatsuba multiplication* Algorithm is used for efficient Mulitplication, *Newton's Method* for the division and *Fast Binary exponentiation* method for Exponentiation. 

Program usage:
    1. make to compile
    2. call executable with following arguments:
       - B<int> to call the program with benchmakr spec. (int signifies number of repetitions)
       - d<int> to output value of <int> decimal places of the square root of two to the console.
       - h<int> to output value of <int> hexadecimal places of the square root of two to the console.
    3. Use --help for additional information.