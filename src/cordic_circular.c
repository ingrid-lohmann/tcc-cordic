#include <stddef.h>
#include <math.h>
#include "cordic_circular.h"

/* Motor 1: Modo de Rotação (força z -> 0) */
CordicVector cordic_circular_rotate(CordicVector v, int iterations) {
    for (int i = 0; i < iterations; i++) {
        DoubleIEEE x_shift = v.x;
        DoubleIEEE y_shift = v.y;

        CORDIC_SHIFT_DOUBLE(x_shift, i);
        CORDIC_SHIFT_DOUBLE(y_shift, i);

        if (v.z.x >= 0.0) {
            v.x.x = v.x.x - y_shift.x;
            v.y.x = v.y.x + x_shift.x;
            v.z.x = v.z.x - CORDIC_CIRCULAR_LUT[i];
        } else {
            v.x.x = v.x.x + y_shift.x;
            v.y.x = v.y.x - x_shift.x;
            v.z.x = v.z.x + CORDIC_CIRCULAR_LUT[i];
        }
    }
    return v;
}

/*
 * Na vetorizacao circular, o vetor (x, y) e girado ate que y se aproxime de
 * zero. O sinal de y define o sentido de cada rotacao, enquanto o angulo
 * escolhido e somado ao acumulador z.
 *
 * Ao final, y tende a zero, z assume o valor z_0 + arctan(y_0 / x_0) e x
 * corresponde a (1/K) * sqrt(x_0^2 + y_0^2). Para calcular arctan(a), o
 * vetor inicial e definido por x = 1, y = a e z = 0. Nesse caso, o valor
 * acumulado em z converge para arctan(a).
 */
CordicVector cordic_circular_vector(CordicVector v, int iterations) {
    for (int i = 0; i < iterations; i++) {
        DoubleIEEE x_shift = v.x;
        DoubleIEEE y_shift = v.y;

        CORDIC_SHIFT_DOUBLE(x_shift, i);
        CORDIC_SHIFT_DOUBLE(y_shift, i);

        /* Se y < 0, rotaciona no sentido anti-horário (d_i = +1) para aproximar de zero */
        if (v.y.x < 0.0) {
            v.x.x = v.x.x - y_shift.x;
            v.y.x = v.y.x + x_shift.x;
            v.z.x = v.z.x - CORDIC_CIRCULAR_LUT[i];
        } else {
            v.x.x = v.x.x + y_shift.x;
            v.y.x = v.y.x - x_shift.x;
            v.z.x = v.z.x + CORDIC_CIRCULAR_LUT[i];
        }
    }
    return v;
}

/* Seno e Cosseno simultâneos via Rotação */
void cordic_circular_sin_cos(double angle_rad, double *sin_out, double *cos_out, int iterations) {
    CordicVector v;
    v.x.x = CORDIC_K_CIRCULAR;
    v.y.x = 0.0;
    v.z.x = angle_rad;

    CordicVector result = cordic_circular_rotate(v, iterations);

    if (cos_out != NULL) {
        *cos_out = result.x.x;
    }
    if (sin_out != NULL) {
        *sin_out = result.y.x;
    }
}

/* Tangente: sin(x) / cos(x) */
double cordic_circular_tan(double angle_rad, int iterations) {
    double s, c;
    cordic_circular_sin_cos(angle_rad, &s, &c, iterations);

    if (fabs(c) < 1e-15) {
        return (s >= 0.0) ? INFINITY : -INFINITY;
    }

    return s / c;
}

/* Arco-tangente via Vetorização: atan(a) */
double cordic_circular_atan(double value, int iterations) {
    CordicVector v;
    v.x.x = 1.0;
    v.y.x = value;
    v.z.x = 0.0;

    CordicVector result = cordic_circular_vector(v, iterations);

    /* O valor do ângulo acumulado em z corresponde ao arco-tangente */
    return result.z.x;
}