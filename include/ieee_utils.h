#ifndef IEEE_UTILS_H
#define IEEE_UTILS_H

#include <math.h>

#define PI       3.141592653589793238462643383279502884
#define TWO_PI   6.283185307179586476925286766559005768
#define PI_2     1.570796326794896619231321691639751442
#define PI_4     0.785398163397448309615660845819875721
#define E1       2.718281828459045235360287471352662498


typedef union {
    float x; 
    struct {
        unsigned int f : 23; 
        unsigned int E : 8;  
        unsigned int s : 1;  
    } bits;
} FloatIEEE;

typedef union {
    double x; 
    struct {
        unsigned long long f : 52; 
        unsigned long long E : 11; 
        unsigned long long s : 1;  
    } bits;
} DoubleIEEE;

#define InvertSign(y) ((y).bits.s ^= 1)

#define CORDIC_SHIFT_DOUBLE(val, i) do { \
    if ((val).bits.E > (unsigned int)(i)) { \
        (val).bits.E -= (i);             \
    } else {                             \
        (val).bits.E = 0;                \
        (val).bits.f = 0;                \
    }                                    \
} while(0)

/* * CODIGO ANTIGO DESCONTINUADO:
 * * #define MULT_NICE_NUMBER(y, w, k) do { \
 * (w).x = (y).x;                     \
 * (w).bits.E += (k);                 \
 * (w).x += (y).x;                    \
 * } while(0)
 * * #define MULT_NICE_NUMBER_NEG(y, w, k) do { \
 * (w).x = -(y).x;                        \
 * (w).bits.E += (k);                     \
 * (w).x += (y).x;                        \
 * } while(0)
 */

void print_float_components(FloatIEEE val);

void print_double_components(DoubleIEEE val);

#endif