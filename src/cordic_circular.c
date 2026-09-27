#include <stddef.h>
#include <math.h>
#include "cordic_circular.h"
#include "ieee_utils.h"

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
 * A vetorizacao circular gira o vetor (x, y) ate que y se aproxime de zero.
 * O sinal de y determina o sentido da proxima rotacao, e o angulo aplicado e
 * acumulado em z. Ao final, z converge para z_0 + arctan(y_0 / x_0), enquanto
 * x representa (1/K) * sqrt(x_0^2 + y_0^2).
 *
 * Para calcular arctan(a), inicia-se com x = 1, y = a e z = 0. O angulo
 * acumulado em z converge entao para arctan(a).
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
    if (!sin_out && !cos_out) return;

    // 1. Tratamento de NaN e Infinito (IEEE 754)
    if (isnan(angle_rad) || isinf(angle_rad)) {
        if (cos_out != NULL) *cos_out = NAN;
        if (sin_out != NULL) *sin_out = NAN;
        return;
    }

    // 2. Redução periódica para o intervalo [-PI, PI]
    angle_rad = fmod(angle_rad, TWO_PI);
    if (angle_rad > PI) {
        angle_rad -= TWO_PI;
    } else if (angle_rad < -PI) {
        angle_rad += TWO_PI;
    }

    // 3. Mapeamento para [-PI_2, PI_2] com inversão de sinal (2º e 3º quadrantes)
    double sign = 1.0;
    if (angle_rad > PI_2) {
        angle_rad -= PI;
        sign = -1.0;
    } else if (angle_rad < -PI_2) {
        angle_rad += PI;
        sign = -1.0;
    }

    // 4. Execução do CORDIC com ângulo garantidamente em [-PI/2, PI/2]
    CordicVector v;
    v.x.x = CORDIC_K_CIRCULAR;
    v.y.x = 0.0;
    v.z.x = angle_rad;

    CordicVector result = cordic_circular_rotate(v, iterations);

    // 5. Aplicação do sinal referente ao quadrante original
    if (cos_out != NULL) {
        *cos_out = sign * result.x.x;
    }
    if (sin_out != NULL) {
        *sin_out = sign * result.y.x;
    }
}

/* Tangente: sin(x) / cos(x) */
double cordic_circular_tan(double angle_rad, int iterations) {
    /* Tratamento de valores excepcionais segundo IEEE 754 */
    if (isnan(angle_rad) || isinf(angle_rad)) {
        return NAN;
    }

    /* Redução por simetria ímpar: tan(-x) = -tan(x) */
    double sign = 1.0;
    if (angle_rad < 0.0) {
        sign = -1.0;
        angle_rad = -angle_rad;
    }

    /* Redução periódica ao intervalo [0, pi) */
    while (angle_rad >= PI) {
        angle_rad -= PI;
    }

    /* Se o ângulo cair no segundo quadrante, aplica tan(pi - x) = -tan(x) */
    if (angle_rad > PI_2) {
        angle_rad = PI - angle_rad;
        sign = -sign;
    }

    /* Verificação de singularidade assintótica em pi/2 */
    if (fabs(angle_rad - PI_2) < 1e-15) {
        return (sign > 0.0) ? INFINITY : -INFINITY;
    }

    double s, c;

    /* Para angulos acima de pi/4, calcula o complementar alpha = pi/2 - theta.
     * Assim, o CORDIC trabalha em [0, pi/4] e a identidade tan(theta) =
     * cos(alpha) / sin(alpha) fornece o valor da tangente original.
     */
    if (angle_rad > PI_4) {
        double alpha = PI_2 - angle_rad;
        cordic_circular_sin_cos(alpha, &s, &c, iterations);

        if (fabs(s) < 1e-15) {
            return (sign > 0.0) ? INFINITY : -INFINITY;
        }

        return sign * (c / s);
    }

    /* Avaliação direta para theta em [0, pi/4] */
    cordic_circular_sin_cos(angle_rad, &s, &c, iterations);

    if (fabs(c) < 1e-15) {
        return (sign > 0.0) ? INFINITY : -INFINITY;
    }

    return sign * (s / c);
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