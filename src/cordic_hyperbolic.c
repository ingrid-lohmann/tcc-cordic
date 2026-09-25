#include <stddef.h>
#include <math.h>
#include "cordic_hyperbolic.h"

/* Passo unitario hiperbolico para o modo de Rotacao (z -> 0) */
static inline CordicVector cordic_hyperbolic_rotate_step(CordicVector v, int i) {
    DoubleIEEE x_shift = v.x;
    DoubleIEEE y_shift = v.y;

    CORDIC_SHIFT_DOUBLE(x_shift, i);
    CORDIC_SHIFT_DOUBLE(y_shift, i);

    if (v.z.x >= 0.0) {
        v.x.x = v.x.x + y_shift.x;
        v.y.x = v.y.x + x_shift.x;
        v.z.x = v.z.x - CORDIC_HYPERBOLIC_LUT[i];
    } else {
        v.x.x = v.x.x - y_shift.x;
        v.y.x = v.y.x - x_shift.x;
        v.z.x = v.z.x + CORDIC_HYPERBOLIC_LUT[i];
    }
    return v;
}

/* Passo unitario hiperbolico para o modo de Vetorizacao (y -> 0) */
static inline CordicVector cordic_hyperbolic_vector_step(CordicVector v, int i) {
    DoubleIEEE x_shift = v.x;
    DoubleIEEE y_shift = v.y;

    CORDIC_SHIFT_DOUBLE(x_shift, i);
    CORDIC_SHIFT_DOUBLE(y_shift, i);

    /* Se y < 0: soma y_shift para aproximar y de 0 (di = +1) */
    if (v.y.x < 0.0) {
        v.x.x = v.x.x + y_shift.x;
        v.y.x = v.y.x + x_shift.x;
        v.z.x = v.z.x - CORDIC_HYPERBOLIC_LUT[i];
    } else {
        v.x.x = v.x.x - y_shift.x;
        v.y.x = v.y.x - x_shift.x;
        v.z.x = v.z.x + CORDIC_HYPERBOLIC_LUT[i];
    }
    return v;
}

/* Motor 1: Rotação Hiperbólica (z -> 0) */
CordicVector cordic_hyperbolic_rotate(CordicVector v, int iterations) {
    for (int i = 1; i <= iterations; i++) {
        v = cordic_hyperbolic_rotate_step(v, i);

        if (i == 4 || i == 13) {
            v = cordic_hyperbolic_rotate_step(v, i);
        }
    }
    return v;
}

/*
 * Na vetorizacao hiperbolica, as iteracoes reduzem y ate que ele se aproxime
 * de zero. O acumulador z converge para z_0 + atanh(y_0 / x_0), e x passa a
 * representar (1/K) * sqrt(x_0^2 - y_0^2).
 *
 * O CORDIC hiperbolico precisa repetir algumas iteracoes para manter a
 * convergencia. Neste caso, as iteracoes i = 4 e i = 13 sao executadas duas
 * vezes, de acordo com o criterio i = 3k + 1. Outra sequencia de repeticao
 * pode ser usada a partir de i_j = (3^(j+1) - 1) / 2, para j > 1.
 */
CordicVector cordic_hyperbolic_vector(CordicVector v, int iterations) {
    for (int i = 1; i <= iterations; i++) {
        v = cordic_hyperbolic_vector_step(v, i);

        if (i == 4 || i == 13) {
            v = cordic_hyperbolic_vector_step(v, i);
        }
    }
    return v;
}

/* Sinh e Cosh simultâneos via Rotação */
void cordic_hyperbolic_sinh_cosh(double angle_rad, double *sinh_out, double *cosh_out, int iterations) {
    CordicVector v;
    v.x.x = CORDIC_K_HYPERBOLIC;
    v.y.x = 0.0;
    v.z.x = angle_rad;

    CordicVector result = cordic_hyperbolic_rotate(v, iterations);

    if (cosh_out != NULL) {
        *cosh_out = result.x.x;
    }
    if (sinh_out != NULL) {
        *sinh_out = result.y.x;
    }
}

/* Tangente Hiperbólica: sinh(x) / cosh(x) */
double cordic_hyperbolic_tanh(double angle_rad, int iterations) {
    double s, c;
    cordic_hyperbolic_sinh_cosh(angle_rad, &s, &c, iterations);
    return s / c;
}

/* Arco-tangente Hiperbólica via Vetorização: atanh(a) */
double cordic_hyperbolic_atanh(double value, int iterations) {
    if (fabs(value) >= 1.0) {
        return NAN; /* Domínio de atanh é (-1, 1) */
    }

    CordicVector v;
    v.x.x = 1.0;
    v.y.x = value;
    v.z.x = 0.0;

    CordicVector result = cordic_hyperbolic_vector(v, iterations);

    return result.z.x;
}