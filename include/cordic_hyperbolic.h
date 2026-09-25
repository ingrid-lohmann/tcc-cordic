#ifndef CORDIC_HYPERBOLIC_H
#define CORDIC_HYPERBOLIC_H

#include "cordic_types.h"
#include "cordic_lut.h"

/* Motores primitivos aritméticos */
CordicVector cordic_hyperbolic_rotate(CordicVector v, int iterations);
CordicVector cordic_hyperbolic_vector(CordicVector v, int iterations);

/* Funções hiperbólicas públicas */
void cordic_hyperbolic_sinh_cosh(double angle_rad, double *sinh_out, double *cosh_out, int iterations);
double cordic_hyperbolic_tanh(double angle_rad, int iterations);
double cordic_hyperbolic_atanh(double value, int iterations);

#endif