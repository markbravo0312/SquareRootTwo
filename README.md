# SquareRootTwo

This Program calculates n digits of the square root of Two to O(log(n)^2).

A fixed point struct **bignum** is used with optimized arithmetic. *Karatsuba multiplication* Algorithm is used for efficient Mulitplication, *Newton's Method* for the division and *Fast Binary exponentiation* method for Exponentiation. 

Program usage:
1. *make* to compile
2. Call executable with following arguments:
* B*x* to call the program with benchmakr spec. (int signifies number of repetitions)
* d*x* to output value of *x* decimal places of the square root of two to the console.
* h*x* to output value of *x* hexadecimal places of the square root of two to the console.

3. Use --help for additional information.


The program can with 100% accuracy calculate the Square root of two up to 20 million decimal places by use of fast operations:  

By use of following formula: 


<img width="521" alt="sqrtwo" src="https://github.com/markbravo0312/SquareRootTwo/assets/20713934/8dfee91e-5f95-4f84-bcd3-cdb25b53e212">


Fast Matrix Exponentiation is used to compute x<sub>n</sub> and x<sub>n+1</sub> then Newton-Raphson division is used to compute x<sub>n</sub> / x<sub>n+1</sub>. Within the division algorithm itself Karatsuba Multiplication is used for the reciprocal of the divisor. 
