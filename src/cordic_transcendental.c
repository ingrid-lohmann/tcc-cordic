#include <math.h>
#include "cordic_transcendental.h"
#include "cordic_hyperbolic.h"
#include "cordic_lut.h"

/* Calcula ln(2) pela identidade ln(2) = 2 * atanh(1/3). O argumento 1/3
 * fica dentro da faixa de convergencia do CORDIC hiperbolico, sem reducao.
 */
static double get_cordic_ln2(int iterations) {
    return 2.0 * cordic_hyperbolic_atanh(1.0 / 3.0, iterations);
}
/* Decompoe x em k * ln(2) + r para calcular e^r = cosh(r) + sinh(r).
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

    /* Multiplica pelo fator 2^k */
    return ldexp(exp_r, k);
}

/* Usa ln(x) = 2 * atanh((x - 1) / (x + 1)). A decomposicao feita por
 * frexp, x = m * 2^exp_val com m em [0.5, 1.0), permite calcular ln(m) e
 * recompor o resultado como ln(m) + exp_val * ln(2).
 */
double cordic_ln(double x, int iterations) {
    if (isnan(x)) return NAN;

    if (x < 0.0) return NAN;

    if (x == 0.0) return -INFINITY;
    
    if (isinf(x)) return INFINITY;

    if (x <= 0.0) {
        return (x == 0.0) ? -INFINITY : NAN; /* ln nao definido para x <= 0 */
    }

    double ln2 = get_cordic_ln2(iterations);

    int exp_val;
    double m = frexp(x, &exp_val);

    double u = (m - 1.0) / (m + 1.0);
    double ln_m = 2.0 * cordic_hyperbolic_atanh(u, iterations);

    return ln_m + (double)exp_val * ln2;
}

/* A raiz e calculada por vetorizacao hiperbolica. Com x0 = m + 0.25 e
 * y0 = m - 0.25, vale x0^2 - y0^2 = m. A vetorizacao produz
 * (1/K) * sqrt(m); a multiplicacao por K_HYPERBOLIC corrige esse ganho.
 *
 * Antes do calculo, x e escrito como m * 4^k, com m em [0.5, 2.0), para
 * manter m em uma faixa adequada. O fator 4^k e aplicado ao resultado.
 */
double cordic_sqrt(double x, int iterations) {
    if (x < 0.0) {
        return NAN; /* Raiz de numero negativo */
    }
    if (x == 0.0) {
        return 0.0;
    }

    int exp_val;
    double m = frexp(x, &exp_val);

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

    /* Desfaz o ganho hiperbolico em x */
    double sqrt_m = result.x.x * CORDIC_K_HYPERBOLIC;

    return ldexp(sqrt_m, k);
}

/* Potenciacao arbitraria x^y = exp(y * ln(x)) */
double cordic_pow(double base, double exp, int iterations) {
    if (base < 0.0) {
        return NAN;
    }
    if (base == 0.0) {
        return (exp > 0.0) ? 0.0 : INFINITY;
    }
    if (exp == 0.0) {
        return 1.0;
    }

    double ln_base = cordic_ln(base, iterations);
    return cordic_exp(exp * ln_base, iterations);
}