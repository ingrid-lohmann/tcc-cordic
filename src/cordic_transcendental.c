#include <math.h>
#include "ieee_utils.h"
#include "cordic_transcendental.h"
#include "cordic_hyperbolic.h"
#include "cordic_lut.h"

/* Calcula ln(2) pela identidade ln(2) = 2 * atanh(1/3). O argumento 1/3
 * fica dentro da faixa de convergência do CORDIC hiperbólico, sem redução.
 */
static double get_cordic_ln2(int iterations) {
    return 2.0 * cordic_hyperbolic_atanh(1.0 / 3.0, iterations);
}

/* ====================================================================
 * Funções Auxiliares: Extração nativa IEEE 754 (substitutas de frexp/ldexp)
 * ==================================================================== */

/* Extrai a mantissa normalizada m em [0.5, 1.0) e o respectivo expoente binário.
 * Equivalente matemático a: m = frexp(x, &exp_val)
 */
static double cordic_frexp(double x, int *exp_val) {
    if (x == 0.0 || isnan(x) || isinf(x)) {
        *exp_val = 0;
        return x;
    }

    DoubleIEEE val;
    val.x = x;

    /* Trata números subnormais multiplicando por 2^52 e ajustando o bias */
    if (val.bits.E == 0) {
        val.x *= 4503599627370496.0; /* 2^52 */
        *exp_val = (int)val.bits.E - 1022 - 52;
    } else {
        *exp_val = (int)val.bits.E - 1022;
    }

    /* Sobrescreve o expoente para -1 no formato IEEE 754 (1022 / 0x3FE).
     * Isso obriga o valor a pertencer ao intervalo [0.5, 1.0). */
    val.bits.E = 1022;

    return val.x;
}

/* Multiplica um valor por 2 elevado a k manipulando diretamente o expoente.
 * Equivalente matemático a: ldexp(x, k)
 */
static double cordic_ldexp(double x, int k) {
    if (x == 0.0 || isnan(x) || isinf(x) || k == 0) {
        return x;
    }

    DoubleIEEE val;
    val.x = x;

    /* Soma algébrica no campo do expoente */
    long long new_E = (long long)val.bits.E + k;

    if (new_E <= 0) {
        /* Underflow ou zona subnormal: simplificação para zero */
        return (val.bits.s == 1) ? -0.0 : 0.0;
    } else if (new_E >= 2047) {
        /* Overflow: resulta em infinito */
        return (val.bits.s == 1) ? -INFINITY : INFINITY;
    }

    val.bits.E = (unsigned long long)new_E;
    return val.x;
}

/* ====================================================================
 * Implementações Transcendentais
 * ==================================================================== */

/* Decompõe x em k * ln(2) + r para calcular e^r = cosh(r) + sinh(r).
 * Depois, recupera e^x multiplicando e^r por 2^k.
 */
double cordic_exp(double x, int iterations) {
    if (isnan(x)) return NAN;
    if (x == 0.0) return 1.0;

    double ln2 = get_cordic_ln2(iterations);

    int k = (int)round(x / ln2);
    double r = x - (double)k * ln2;

    double sh, ch;
    cordic_hyperbolic_sinh_cosh(r, &sh, &ch, iterations);
    double exp_r = ch + sh;

    /* Multiplica pelo fator 2^k sem usar ldexp() */
    return cordic_ldexp(exp_r, k);
}

/* Usa ln(x) = 2 * atanh((x - 1) / (x + 1)). A decomposição feita por
 * cordic_frexp, x = m * 2^exp_val com m em [0.5, 1.0), permite calcular ln(m) e
 * recompor o resultado como ln(m) + exp_val * ln(2).
 */
double cordic_ln(double x, int iterations) {
    if (isnan(x) || x < 0.0) return NAN;
    if (x == 0.0) return -INFINITY;
    if (isinf(x)) return INFINITY;

    double ln2 = get_cordic_ln2(iterations);

    int exp_val;
    /* Extração da mantissa via máscara de bits em vez de math.h */
    double m = cordic_frexp(x, &exp_val);

    double u = (m - 1.0) / (m + 1.0);
    double ln_m = 2.0 * cordic_hyperbolic_atanh(u, iterations);

    return ln_m + (double)exp_val * ln2;
}

/* A raiz é calculada por vetorização hiperbólica. Com x0 = m + 0.25 e
 * y0 = m - 0.25, vale x0^2 - y0^2 = m. A vetorização produz
 * (1/K) * sqrt(m); a multiplicação por K_HYPERBOLIC corrige esse ganho.
 *
 * Antes do cálculo, x é escrito como m * 4^k, com m em [0.5, 2.0), para
 * manter m em uma faixa adequada. O fator 2^k é aplicado ao resultado final.
 */
double cordic_sqrt(double x, int iterations) {
    if (x < 0.0) return NAN;
    if (x == 0.0) return 0.0;

    int exp_val;
    double m = cordic_frexp(x, &exp_val);

    int k;
    if (exp_val % 2 != 0) {
        m *= 2.0;
        k = (exp_val - 1) / 2;
    } else {
        k = exp_val / 2;
    }

    CordicVector v;
    v.x.x = m + 0.25;
    v.y.x = m - 0.25;
    v.z.x = 0.0;

    CordicVector result = cordic_hyperbolic_vector(v, iterations);

    /* Desfaz o ganho hiperbólico em x */
    double sqrt_m = result.x.x * CORDIC_K_HYPERBOLIC;

    /* Recomposição final via manipulação de bits em vez de ldexp() */
    return cordic_ldexp(sqrt_m, k);
}

/* Potenciação arbitrária x^y = exp(y * ln(x)) */
double cordic_pow(double base, double exp, int iterations) {
    if (base < 0.0) return NAN;
    if (base == 0.0) return (exp > 0.0) ? 0.0 : INFINITY;
    if (exp == 0.0) return 1.0;

    double ln_base = cordic_ln(base, iterations);
    return cordic_exp(exp * ln_base, iterations);
}