#include <stddef.h>
#include "cordic_circular.h"
#include "ieee_utils.h"

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

CordicVector cordic_circular_vector(CordicVector v, int iterations) {
    for (int i = 0; i < iterations; i++) {
        DoubleIEEE x_shift = v.x;
        DoubleIEEE y_shift = v.y;

        CORDIC_SHIFT_DOUBLE(x_shift, i);
        CORDIC_SHIFT_DOUBLE(y_shift, i);

        /* y < 0: aproxima de zero girando no sentido anti-horário */
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

void cordic_circular_sin_cos(double angle_rad, double *sin_out, double *cos_out, int iterations) {
    if (!sin_out && !cos_out) return;

    if (cordic_isnan(angle_rad) || cordic_isinf(angle_rad)) {
        if (cos_out != NULL) *cos_out = cordic_nan();
        if (sin_out != NULL) *sin_out = cordic_nan();
        return;
    }

    /* Redução periódica ao intervalo principal sem usar fmod da math.h */
    angle_rad = cordic_fmod_twopi(angle_rad);
    if (angle_rad > PI) {
        angle_rad -= TWO_PI;
    } else if (angle_rad < -PI) {
        angle_rad += TWO_PI;
    }

    double sign = 1.0;
    if (angle_rad > PI_2) {
        angle_rad -= PI;
        sign = -1.0;
    } else if (angle_rad < -PI_2) {
        angle_rad += PI;
        sign = -1.0;
    }

    CordicVector v;
    v.x.x = CORDIC_K_CIRCULAR;
    v.y.x = 0.0;
    v.z.x = angle_rad;

    CordicVector result = cordic_circular_rotate(v, iterations);

    if (cos_out != NULL) *cos_out = sign * result.x.x;
    if (sin_out != NULL) *sin_out = sign * result.y.x;
}

double cordic_circular_tan(double angle_rad, int iterations) {
    if (cordic_isnan(angle_rad) || cordic_isinf(angle_rad)) return cordic_nan();

    double sign = 1.0;
    if (angle_rad < 0.0) {
        sign = -1.0;
        angle_rad = -angle_rad;
    }

    while (angle_rad >= PI) angle_rad -= PI;

    if (angle_rad > PI_2) {
        angle_rad = PI - angle_rad;
        sign = -sign;
    }

    /* Manipulação de bit a bit de valor absoluto no lugar de fabs() */
    if (cordic_fabs(angle_rad - PI_2) < 1e-15) {
        return (sign > 0.0) ? cordic_inf(1) : cordic_inf(-1);
    }

    double s, c;

    /* Para ângulos > pi/4, trabalha com o complementar alpha = pi/2 - theta */
    if (angle_rad > PI_4) {
        double alpha = PI_2 - angle_rad;
        cordic_circular_sin_cos(alpha, &s, &c, iterations);
        
        if (cordic_fabs(s) < 1e-15) return (sign > 0.0) ? cordic_inf(1) : cordic_inf(-1);
        return sign * (c / s);
    }

    cordic_circular_sin_cos(angle_rad, &s, &c, iterations);
    
    if (cordic_fabs(c) < 1e-15) return (sign > 0.0) ? cordic_inf(1) : cordic_inf(-1);
    return sign * (s / c);
}

double cordic_circular_atan(double value, int iterations) {
    CordicVector v;
    v.x.x = 1.0;
    v.y.x = value;
    v.z.x = 0.0;

    CordicVector result = cordic_circular_vector(v, iterations);
    return result.z.x;
}