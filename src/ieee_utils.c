#include <stdio.h>
#include "ieee_utils.h"

void print_float_components(FloatIEEE val) {
    printf("\n--- Analise IEEE 754 ---");
    printf("\n Valor real: %f", val.x);
    printf("\n Sinal:      %d", val.bits.s);
    printf("\n Expoente:   %d", val.bits.E);
    printf("\n Mantissa:   %u", val.bits.f);
    printf("\n Fracao:     %f", (float)val.bits.f / pow(2, 23));
    printf("\n------------------------\n");
}