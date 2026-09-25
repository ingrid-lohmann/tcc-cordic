#include <math.h>
#include "cordic_transcendental.h"
#include "cordic_hyperbolic.h"
#include "cordic_lut.h"

/* Armazena o valor de ln(2) computado dinamicamente via CORDIC */
static double cached_ln2 = 0.0;

/*
 * Calcula ln(2) a partir da identidade
 * ln(2) = 2 * atanh((2 - 1) / (2 + 1)) = 2 * atanh(1/3).
 * Como 1/3 e menor que 0.8069, o argumento esta dentro da faixa de
 * convergencia usada pelo CORDIC hiperbolico e nao precisa ser reduzido.
 */
static double get_cordic_ln2(int iterations) {
    if (cached_ln2 == 0.0) {
        cached_ln2 = 2.0 * cordic_hyperbolic_atanh(1.0 / 3.0, iterations);
    }
    return cached_ln2;
}

/*
 * Calcula e^x usando e^r = cosh(r) + sinh(r). Antes da chamada ao CORDIC,
 * x e separado em x = k * ln(2) + r. O resultado e entao reconstruido como
 * 2^k * e^r, mantendo r pequeno para favorecer a convergencia.
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

/*
 * Calcula o logaritmo pela identidade ln(x) = 2 * atanh((x - 1) / (x + 1)).
 * Para manter o argumento em uma faixa adequada, x e decomposto pela funcao
 * frexp como x = m * 2^exp_val, com m em [0.5, 1.0). Assim, o resultado e
 * obtido por ln(x) = ln(m) + exp_val * ln(2).
 */
double cordic_ln(double x, int iterations) {
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

/*
 * A raiz quadrada usa a relacao hiperbolica entre x0 = x + 0.25 e
 * y0 = x - 0.25. A diferenca x0^2 - y0^2 e igual a x, portanto a
 * vetorizacao produz (1/K) * sqrt(x) em x. O produto por K_HYPERBOLIC
 * remove esse ganho.
 *
 * Antes da vetorizacao, x e escrito na forma m * 4^k, com m em [0.5, 2.0).
 * O calculo e feito sobre m e o fator 4^k e aplicado ao final.
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