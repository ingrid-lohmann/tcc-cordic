#ifndef CORDIC_CIRCULAR_H
#define CORDIC_CIRCULAR_H

#include "cordic_types.h"
#include "cordic_lut.h"

/* Motores primitivos aritméticos */
CordicVector cordic_circular_rotate(CordicVector v, int iterations);
CordicVector cordic_circular_vector(CordicVector v, int iterations);

/* Funções transcendentais públicas */
void cordic_circular_sin_cos(double angle_rad, double *sin_out, double *cos_out, int iterations);
double cordic_circular_tan(double angle_rad, int iterations);
double cordic_circular_atan(double value, int iterations);

#endif