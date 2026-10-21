#include <stddef.h>
#include "cordic_hyperbolic.h"
#include "ieee_utils.h"

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

static inline CordicVector cordic_hyperbolic_vector_step(CordicVector v, int i) {
    DoubleIEEE x_shift = v.x;
    DoubleIEEE y_shift = v.y;

    CORDIC_SHIFT_DOUBLE(x_shift, i);
    CORDIC_SHIFT_DOUBLE(y_shift, i);

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

CordicVector cordic_hyperbolic_rotate(CordicVector v, int iterations) {
    for (int i = 1; i <= iterations; i++) {
        v = cordic_hyperbolic_rotate_step(v, i);
        
        if (i == 4 || i == 13) {
            v = cordic_hyperbolic_rotate_step(v, i);
        }
    }
    return v;
}

CordicVector cordic_hyperbolic_vector(CordicVector v, int iterations) {
    for (int i = 1; i <= iterations; i++) {
        v = cordic_hyperbolic_vector_step(v, i);
        
        /* Iterações obrigatórias para manter convergência hiperbólica */
        if (i == 4 || i == 13) {
            v = cordic_hyperbolic_vector_step(v, i);
        }
    }
    return v;
}

void cordic_hyperbolic_sinh_cosh(double angle_rad, double *sinh_out, double *cosh_out, int iterations) {
    CordicVector v;
    v.x.x = CORDIC_K_HYPERBOLIC;
    v.y.x = 0.0;
    v.z.x = angle_rad;

    CordicVector result = cordic_hyperbolic_rotate(v, iterations);

    if (cosh_out != NULL) *cosh_out = result.x.x;
    if (sinh_out != NULL) *sinh_out = result.y.x;
}

double cordic_hyperbolic_tanh(double angle_rad, int iterations) {
    double s, c;
    cordic_hyperbolic_sinh_cosh(angle_rad, &s, &c, iterations);
    return s / c;
}

double cordic_hyperbolic_atanh(double value, int iterations) {
    /* Manipulação de bit a bit no lugar de fabs() */
    if (cordic_fabs(value) >= 1.0) return cordic_nan();

    CordicVector v;
    v.x.x = 1.0;
    v.y.x = value;
    v.z.x = 0.0;

    CordicVector result = cordic_hyperbolic_vector(v, iterations);
    return result.z.x;
}