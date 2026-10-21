#include <stdio.h>
#include "ieee_utils.h"

void print_float_components(FloatIEEE val) {
    printf("\n--- Analise IEEE 754 ---");
    printf("\n Valor real: %f", val.x);
    printf("\n Sinal:      %d", val.bits.s);
    printf("\n Expoente:   %d", val.bits.E);
    printf("\n Mantissa:   %u", val.bits.f);
    /* 2^23 = 8388608.0 */
    printf("\n Fracao:     %f", (float)val.bits.f / 8388608.0f);
    printf("\n------------------------\n");
}

void print_double_components(DoubleIEEE val) {
    printf("\n--- Analise IEEE 754 ---");
    printf("\n Valor real: %f", val.x);
    printf("\n Sinal:      %llu", (unsigned long long)val.bits.s);
    printf("\n Expoente:   %llu", (unsigned long long)val.bits.E);
    printf("\n Mantissa:   %llu", (unsigned long long)val.bits.f);
    /* 2^52 = 4503599627370496.0 */
    printf("\n Fracao:     %.17f", (double)val.bits.f / 4503599627370496.0);
    printf("\n------------------------\n");
}