#include "ieee_utils.h"
#include "cordic_transcendental.h"
#include "cordic_hyperbolic.h"
#include "cordic_lut.h"

static double get_cordic_ln2(int iterations) {
    return 2.0 * cordic_hyperbolic_atanh(1.0 / 3.0, iterations);
}

static double cordic_frexp(double x, int *exp_val) {
    if (x == 0.0 || cordic_isnan(x) || cordic_isinf(x)) {
        *exp_val = 0;
        return x;
    }

    DoubleIEEE val;
    val.x = x;

    /* Tratamento de números subnormais escalando o bias */
    if (val.bits.E == 0) {
        val.x *= 4503599627370496.0; /* 2^52 */
        *exp_val = (int)val.bits.E - 1022 - 52;
    } else {
        *exp_val = (int)val.bits.E - 1022;
    }

    val.bits.E = 1022;
    return val.x;
}

static double cordic_ldexp(double x, int k) {
    if (x == 0.0 || cordic_isnan(x) || cordic_isinf(x) || k == 0) {
        return x;
    }

    DoubleIEEE val;
    val.x = x;
    
    long long new_E = (long long)val.bits.E + k;

    if (new_E <= 0) {
        return (val.bits.s == 1) ? -0.0 : 0.0;
    } else if (new_E >= 2047) {
        return cordic_inf(val.bits.s == 1 ? -1 : 1);
    }

    val.bits.E = (unsigned long long)new_E;
    return val.x;
}

double cordic_exp(double x, int iterations) {
    if (cordic_isnan(x)) return cordic_nan();
    if (x == 0.0) return 1.0;

    double ln2 = get_cordic_ln2(iterations);

    /* O round da math.h foi substituído pelo cordic_round nativo */
    int k = (int)cordic_round(x / ln2);
    double r = x - (double)k * ln2;

    double sh, ch;
    cordic_hyperbolic_sinh_cosh(r, &sh, &ch, iterations);
    
    return cordic_ldexp(ch + sh, k);
}

double cordic_ln(double x, int iterations) {
    if (cordic_isnan(x) || x < 0.0) return cordic_nan();
    if (x == 0.0) return cordic_inf(-1);
    if (cordic_isinf(x)) return cordic_inf(1);

    double ln2 = get_cordic_ln2(iterations);
    int exp_val;
    double m = cordic_frexp(x, &exp_val);

    double u = (m - 1.0) / (m + 1.0);
    double ln_m = 2.0 * cordic_hyperbolic_atanh(u, iterations);

    return ln_m + (double)exp_val * ln2;
}

double cordic_sqrt(double x, int iterations) {
    if (x < 0.0) return cordic_nan();
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
    
    /* Desfaz o ganho de vetorização hiperbólica */
    double sqrt_m = result.x.x * CORDIC_K_HYPERBOLIC;

    return cordic_ldexp(sqrt_m, k);
}

double cordic_pow(double base, double exp, int iterations) {
    if (base < 0.0) return cordic_nan();
    if (base == 0.0) return (exp > 0.0) ? 0.0 : cordic_inf(1);
    if (exp == 0.0) return 1.0;

    double ln_base = cordic_ln(base, iterations);
    return cordic_exp(exp * ln_base, iterations);
}